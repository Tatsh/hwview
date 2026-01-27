// SPDX-License-Identifier: MIT
#include <QtConcurrent/QtConcurrent>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QRegularExpression>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QFontDatabase>
#include <QtGui/QShortcut>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>

#include "devicecache.h"
#include "driverdetailsdialog.h"
#include "propertiesdialog.h"
#include "systeminfo.h"

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent), deviceInfo_(nullptr), eventsModel_(nullptr) {
    setupUi(this);
    setMinimumSize(493, 502);

    // Make General tab value labels selectable
    labelDeviceTypeValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelManufacturerValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelLocationValue->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Make device name labels selectable on all tabs
    labelDeviceNameGeneral->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelDeviceNameDriver->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelDeviceNameDetails->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelDeviceNameEvents->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Setup property mappings for Details tab (platform-specific)
    propertyMappings_ = getDevicePropertyMappings();

    // Connect property dropdown
    connect(comboBoxDetailsProperty,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertiesDialog::onPropertySelectionChanged);

    // Setup events table model
    eventsModel_ = new QStandardItemModel(this);
    eventsModel_->setHorizontalHeaderLabels({tr("Timestamp"), tr("Event")});
    tableViewEvents->setModel(eventsModel_);
    tableViewEvents->horizontalHeader()->setStretchLastSection(true);
    tableViewEvents->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewEvents->setSelectionMode(QAbstractItemView::SingleSelection);

    // Connect event selection
    connect(tableViewEvents->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &PropertiesDialog::onEventSelectionChanged);

    // Connect driver details button
    connect(buttonDriverDetails,
            &QPushButton::clicked,
            this,
            &PropertiesDialog::onDriverDetailsClicked);

    // Connect disable device button
    connect(buttonDisableDevice,
            &QPushButton::clicked,
            this,
            &PropertiesDialog::onDisableDeviceClicked);

    // Connect view all events button
    connect(buttonViewAllEvents,
            &QPushButton::clicked,
            this,
            &PropertiesDialog::onViewAllEventsClicked);

    // Shortcut: Ctrl+P copies device path when on General tab
    auto *copyPathShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P), this);
    copyPathShortcut->setContext(Qt::WindowShortcut);
    connect(copyPathShortcut, &QShortcut::activated, this, &PropertiesDialog::onCopyDevicePath);
}

PropertiesDialog::~PropertiesDialog() {
    // Cancel any pending async operations
    if (eventsWatcher_) {
        eventsWatcher_->disconnect();
        eventsWatcher_->cancel();
        eventsWatcher_->waitForFinished();
        delete eventsWatcher_;
        eventsWatcher_ = nullptr;
    }
    if (driverInfoWatcher_) {
        driverInfoWatcher_->disconnect();
        driverInfoWatcher_->cancel();
        driverInfoWatcher_->waitForFinished();
        delete driverInfoWatcher_;
        driverInfoWatcher_ = nullptr;
    }
    if (resourcesWatcher_) {
        resourcesWatcher_->disconnect();
        resourcesWatcher_->cancel();
        resourcesWatcher_->waitForFinished();
        delete resourcesWatcher_;
        resourcesWatcher_ = nullptr;
    }
}

void PropertiesDialog::setDeviceSyspath(const QString &syspath) {
    syspath_ = syspath;
    deviceInfo_ = DeviceCache::instance().deviceBySyspath(syspath);

    if (!deviceInfo_) {
        return;
    }

    // Check if this is the Computer root entry
    auto computerEntry = isComputerEntry(syspath);

    QString deviceName;
    if (computerEntry) {
        deviceName = getComputerDisplayName();
    } else {
        deviceName = getDeviceDisplayName(*deviceInfo_);
    }

    // Set device name labels on all tabs
    labelDeviceNameGeneral->setText(deviceName);
    labelDeviceNameDetails->setText(deviceName);
    labelDeviceNameDriver->setText(deviceName);
    labelDeviceNameEvents->setText(deviceName);

    // Set window title
    setWindowTitle(tr("%1 Properties").arg(deviceName));

    if (computerEntry) {
        // Special handling for Computer entry
        labelDeviceTypeValue->setText(tr("Computer"));
        labelManufacturerValue->setText(tr("(Standard computers)"));
        labelLocationValue->setText(tr("Unknown"));
        labelLocationValue->setToolTip(QString());

        // Hide Events and Details tabs
        auto detailsIndex = tabWidget->indexOf(tabDetails);
        if (detailsIndex >= 0) {
            tabWidget->removeTab(detailsIndex);
        }
        auto eventsIndex = tabWidget->indexOf(tab); // Events tab is named "tab" in UI
        if (eventsIndex >= 0) {
            tabWidget->removeTab(eventsIndex);
        }

        // Populate only General and Driver tabs
        populateDriverTab();
    } else {
        // Populate all tabs
        populateGeneralTab();
        populateDriverTab();
        populateDetailsTab();
        populateEventsTab();

        // Create Resources tab if device has resources
        createResourcesTab();
    }
}

void PropertiesDialog::setCategoryIcon(const QIcon &icon) {
    categoryIcon_ = icon;
    auto pixmap = icon.pixmap(32, 32);

    // Set icon on all tabs
    labelIconGeneral->setPixmap(pixmap);
    labelIconDriver->setPixmap(pixmap);
    labelIconDetails->setPixmap(pixmap);
    labelIconEvents->setPixmap(pixmap);
}

void PropertiesDialog::populateGeneralTab() {
    if (!deviceInfo_)
        return;

    // Device type - use category name matching "Devices by type" view
    auto deviceType = getDeviceCategory();
    if (!deviceType.isEmpty()) {
        labelDeviceTypeValue->setText(deviceType);
    }

    // Manufacturer
    auto manufacturer = getDeviceManufacturer(*deviceInfo_);
    if (manufacturer.isEmpty()) {
        // For storage volumes, show N/A; for others, show Unknown
        if (deviceInfo_->category() == DeviceCategory::StorageVolumes) {
            manufacturer = tr("N/A");
        } else {
            manufacturer = tr("Unknown");
        }
    }
    labelManufacturerValue->setText(manufacturer);

    // Location - translate syspath to human-readable form
    auto location = translateDevicePath(deviceInfo_->syspath());
    if (location.isEmpty()) {
        location = tr("Unknown");
    }
    labelLocationValue->setText(location);
    labelLocationValue->setToolTip(deviceInfo_->syspath());

    // Device status - if the device exists and has a driver, it's working
    auto driver = deviceInfo_->driver();
    if (!driver.isEmpty()) {
        textEditDeviceStatus->setPlainText(tr("This device is working properly."));
    } else {
        textEditDeviceStatus->setPlainText(tr("No driver installed for this device."));
    }
}

void PropertiesDialog::populateDriverTab() {
    if (!deviceInfo_)
        return;

    auto driverName = deviceInfo_->driver();

    if (!hasDriverInfo(*deviceInfo_)) {
        labelDriverProviderValue->setText(tr("(No driver)"));
        labelDigitalSignerValue->setText(tr("N/A"));
        labelDriverVersionValue->setText(tr("N/A"));
        labelDriverDateValue->setText(tr("N/A"));
        buttonDriverDetails->setEnabled(false);
        return;
    }

    // Show loading state
    labelDriverProviderValue->setText(tr("Loading..."));
    labelDriverVersionValue->setText(tr("Loading..."));
    labelDigitalSignerValue->setText(tr("Loading..."));
    labelDriverDateValue->setText(tr("Loading..."));
    buttonDriverDetails->setEnabled(false);

    // Cancel any previous watcher
    if (driverInfoWatcher_) {
        driverInfoWatcher_->disconnect();
        driverInfoWatcher_->cancel();
        driverInfoWatcher_->deleteLater();
    }

    driverInfoWatcher_ = new QFutureWatcher<BasicDriverInfo>(this);
    connect(driverInfoWatcher_,
            &QFutureWatcher<BasicDriverInfo>::finished,
            this,
            &PropertiesDialog::onDriverInfoLoaded);

    driverInfoWatcher_->setFuture(
        QtConcurrent::run([driverName]() { return getBasicDriverInfo(driverName); }));
}

void PropertiesDialog::onDriverInfoLoaded() {
    if (!driverInfoWatcher_) {
        return;
    }

    BasicDriverInfo info = driverInfoWatcher_->result();
    driverInfoWatcher_->deleteLater();
    driverInfoWatcher_ = nullptr;

    labelDriverProviderValue->setText(info.provider);
    labelDriverVersionValue->setText(info.version);
    labelDigitalSignerValue->setText(info.signer);
    labelDriverDateValue->setText(info.date);
    buttonDriverDetails->setEnabled(info.hasDriverFiles);

    // Enable disable button only if driver has files and is not built-in
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    buttonDisableDevice->setEnabled(info.hasDriverFiles && !info.isBuiltin);
#endif
}

QString PropertiesDialog::getDeviceCategory() {
    if (!deviceInfo_) {
        return {};
    }

    return getCategoryDisplayName(deviceInfo_->category(), deviceInfo_->subsystem());
}

void PropertiesDialog::populateDetailsTab() {
    if (!deviceInfo_)
        return;

    // Clear the combo box and rebuild with only properties that have values
    comboBoxDetailsProperty->clear();

    for (const auto &mapping : propertyMappings_) {
        QString value;

        // Handle special properties that aren't direct property lookups
        if (mapping.propertyKey == PropertyKeys::deviceDescription()) {
            value = deviceInfo_->name();
        } else if (mapping.propertyKey == PropertyKeys::syspath()) {
            value = deviceInfo_->syspath();
        } else if (mapping.propertyKey == PropertyKeys::parentSyspath()) {
            value = deviceInfo_->parentSyspath();
        } else if (mapping.propertyKey == PropertyKeys::children()) {
            // Skip children for now - would require additional lookup
            continue;
        } else if (mapping.propertyKey == PropertyKeys::compatibleIds()) {
            // Skip for now
            continue;
        } else if (mapping.propertyKey == PropertyKeys::mountPoint()) {
            value = getMountPoint(deviceInfo_->devnode());
        } else {
            value = deviceInfo_->propertyValue(mapping.propertyKey.toLocal8Bit().constData());
        }

        // Only add property to dropdown if it has a value
        if (!value.isEmpty()) {
            comboBoxDetailsProperty->addItem(mapping.displayName, mapping.propertyKey);
        }
    }

    // Trigger initial selection
    if (comboBoxDetailsProperty->count() > 0) {
        onPropertySelectionChanged(0);
    }
}

void PropertiesDialog::onPropertySelectionChanged(int index) {
    listWidgetDetailsPropertyValue->clear();

    if (index < 0 || !deviceInfo_)
        return;

    auto propertyKey = comboBoxDetailsProperty->currentData().toString();
    QString value;

    // Handle special properties
    if (propertyKey == PropertyKeys::deviceDescription()) {
        value = deviceInfo_->name();
    } else if (propertyKey == PropertyKeys::syspath()) {
        value = deviceInfo_->syspath();
    } else if (propertyKey == PropertyKeys::parentSyspath()) {
        value = deviceInfo_->parentSyspath();
    } else if (propertyKey == PropertyKeys::mountPoint()) {
        value = getMountPoint(deviceInfo_->devnode());
    } else {
        value = deviceInfo_->propertyValue(propertyKey.toLocal8Bit().constData());
    }

    if (!value.isEmpty()) {
        // For multi-value properties, split by newline or semicolon
        if (value.contains(QStringLiteral("\n"))) {
            auto values = value.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
            for (const auto &v : values) {
                listWidgetDetailsPropertyValue->addItem(v.trimmed());
            }
        } else if (value.contains(QStringLiteral(";"))) {
            auto values = value.split(QStringLiteral(";"), Qt::SkipEmptyParts);
            for (const auto &v : values) {
                listWidgetDetailsPropertyValue->addItem(v.trimmed());
            }
        } else {
            listWidgetDetailsPropertyValue->addItem(value);
        }
    }

    // Add Windows-style hardware IDs if applicable (on non-Windows platforms)
    auto hardwareIds = convertToHardwareIds(propertyKey, value);
    for (const auto &hwId : hardwareIds) {
        listWidgetDetailsPropertyValue->addItem(hwId);
    }
}

void PropertiesDialog::populateEventsTab() {
    if (!deviceInfo_)
        return;

    eventsModel_->removeRows(0, eventsModel_->rowCount());
    textEditEventsInfo->setPlainText(tr("Loading events..."));
    buttonViewAllEvents->setEnabled(false);

    // Cancel any previous watcher before starting a new one
    if (eventsWatcher_) {
        eventsWatcher_->disconnect();
        eventsWatcher_->cancel();
        eventsWatcher_->waitForFinished();
        delete eventsWatcher_;
    }

    // Run queryDeviceEvents() asynchronously to avoid blocking the UI
    eventsWatcher_ = new QFutureWatcher<QStringList>(this);
    connect(eventsWatcher_,
            &QFutureWatcher<QStringList>::finished,
            this,
            &PropertiesDialog::onEventsLoaded);

    // Build query from device info using platform-appropriate properties
    DeviceEventQuery query = buildEventQuery(*deviceInfo_);

    auto future = QtConcurrent::run([query]() -> QStringList { return queryDeviceEvents(query); });
    eventsWatcher_->setFuture(future);
}

void PropertiesDialog::onEventsLoaded() {
    if (!eventsWatcher_)
        return;

    allEvents_ = eventsWatcher_->result();
    delete eventsWatcher_;
    eventsWatcher_ = nullptr;

    if (allEvents_.isEmpty()) {
        textEditEventsInfo->setPlainText(tr("No events found."));
        buttonViewAllEvents->setEnabled(false);
        return;
    }

    // Show only first 5 events in the table
    auto displayCount = qMin(qsizetype{5}, allEvents_.size());
    for (decltype(displayCount) i = 0; i < displayCount; ++i) {
        ParsedEvent event = parseEventLine(allEvents_.at(i));

        QList<QStandardItem *> row;
        row << new QStandardItem(event.timestamp);
        row << new QStandardItem(event.message);
        eventsModel_->appendRow(row);
    }

    // Enable "View All Events" button only if there are more than 5 events
    buttonViewAllEvents->setEnabled(allEvents_.size() > 5);

    // Resize columns
    tableViewEvents->resizeColumnsToContents();

    // Select the first event
    auto firstIndex = eventsModel_->index(0, 0);
    tableViewEvents->setCurrentIndex(firstIndex);
    onEventSelectionChanged(firstIndex, QModelIndex());
}

void PropertiesDialog::onEventSelectionChanged(const QModelIndex &current,
                                               const QModelIndex &previous) {
    Q_UNUSED(previous)

    if (!current.isValid()) {
        textEditEventsInfo->clear();
        return;
    }

    // Get the full message from the selected row
    auto row = current.row();
    QStandardItem *timestampItem = eventsModel_->item(row, 0);
    QStandardItem *messageItem = eventsModel_->item(row, 1);

    if (timestampItem && messageItem) {
        auto info = QStringLiteral("%1\n\n%2").arg(timestampItem->text()).arg(messageItem->text());
        textEditEventsInfo->setPlainText(info);
    }
}

void PropertiesDialog::onDriverDetailsClicked() {
    if (!deviceInfo_)
        return;

    auto driver = deviceInfo_->driver();
    if (driver.isEmpty()) {
        return;
    }

    DriverDetailsDialog dialog(this);
    dialog.setCategoryIcon(categoryIcon_);
    dialog.setDriverName(driver);
    dialog.exec();
}

void PropertiesDialog::onDisableDeviceClicked() {
    if (!deviceInfo_)
        return;

    auto driver = deviceInfo_->driver();
    if (driver.isEmpty()) {
        return;
    }

    // Build the platform-specific unload command
    QString command;
#ifdef Q_OS_LINUX
    command = QStringLiteral("modprobe -r %1").arg(driver);
#elif defined(Q_OS_MACOS)
    command = QStringLiteral("kextunload -b %1").arg(driver);
#else
    return; // Not supported on other platforms
#endif

    // Create dialog
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Disable Device"));
    dialog.setMinimumWidth(400);

    auto *layout = new QVBoxLayout(&dialog);

    auto *infoLabel = new QLabel(tr("To disable this device, run the following command:"), &dialog);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    layout->addSpacing(10);

    // Command display in fixed-width font
    auto *commandEdit = new QLineEdit(command, &dialog);
    commandEdit->setReadOnly(true);
    commandEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    commandEdit->selectAll();
    layout->addWidget(commandEdit);

    layout->addSpacing(10);

    // Notes about root privileges, dependencies, and potential failures
    auto *notesLabel = new QLabel(
        tr("Note: This command most likely requires root privileges. Additionally, drivers may "
           "have dependencies that must be unloaded first. The command may fail if the device is "
           "in use (e.g., Wayland or X11 using the GPU)."),
        &dialog);
    notesLabel->setWordWrap(true);
    layout->addWidget(notesLabel);

    layout->addSpacing(10);

    // Copy button and close button
    auto *buttonLayout = new QHBoxLayout();
    auto *copyButton = new QPushButton(tr("Copy to Clipboard"), &dialog);
    connect(copyButton, &QPushButton::clicked, &dialog, [command]() {
        QApplication::clipboard()->setText(command);
    });
    buttonLayout->addWidget(copyButton);

    buttonLayout->addStretch();

    auto *closeButton = new QPushButton(tr("Close"), &dialog);
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    buttonLayout->addWidget(closeButton);

    layout->addLayout(buttonLayout);

    dialog.exec();
}

void PropertiesDialog::onViewAllEventsClicked() {
    if (allEvents_.isEmpty()) {
        return;
    }

    // Create a dialog to show all events
    QDialog dialog(this);
    dialog.setWindowTitle(tr("All Events - %1").arg(deviceInfo_ ? deviceInfo_->name() : QString()));
    dialog.resize(700, 500);

    auto *layout = new QVBoxLayout(&dialog);

    // Create table view for all events
    auto *tableView = new QTableView(&dialog);
    auto *model = new QStandardItemModel(&dialog);
    model->setHorizontalHeaderLabels({tr("Timestamp"), tr("Event")});

    for (const auto &eventLine : allEvents_) {
        ParsedEvent event = parseEventLine(eventLine);

        QList<QStandardItem *> row;
        row << new QStandardItem(event.timestamp);
        row << new QStandardItem(event.message);
        model->appendRow(row);
    }

    tableView->setModel(model);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->resizeColumnsToContents();

    layout->addWidget(tableView);

    // Add close button
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    dialog.exec();
}

void PropertiesDialog::onCopyDevicePath() {
    // Only copy when General tab is active
    if (tabWidget->currentWidget() != tabGeneral) {
        return;
    }

    if (!syspath_.isEmpty()) {
        QApplication::clipboard()->setText(syspath_);
    }
}

void PropertiesDialog::createResourcesTab() {
    // Remove existing resources tab if present
    if (resourcesTab_) {
        auto index = tabWidget->indexOf(resourcesTab_);
        if (index >= 0) {
            tabWidget->removeTab(index);
        }
        delete resourcesTab_;
        resourcesTab_ = nullptr;
    }

    if (!deviceInfo_) {
        return;
    }

    // Cancel any previous watcher
    if (resourcesWatcher_) {
        resourcesWatcher_->disconnect();
        resourcesWatcher_->cancel();
        resourcesWatcher_->deleteLater();
    }

    // Fetch resources asynchronously
    resourcesWatcher_ = new QFutureWatcher<QList<ResourceInfo>>(this);
    connect(resourcesWatcher_,
            &QFutureWatcher<QList<ResourceInfo>>::finished,
            this,
            &PropertiesDialog::onResourcesLoaded);

    auto syspath = deviceInfo_->syspath();
    auto driverName = deviceInfo_->driver();
    resourcesWatcher_->setFuture(QtConcurrent::run(
        [syspath, driverName]() { return getDeviceResources(syspath, driverName); }));
}

void PropertiesDialog::onResourcesLoaded() {
    if (!resourcesWatcher_ || !deviceInfo_) {
        return;
    }

    QList<ResourceInfo> resources = resourcesWatcher_->result();
    resourcesWatcher_->deleteLater();
    resourcesWatcher_ = nullptr;

    // Only create the tab if there are resources
    if (resources.isEmpty()) {
        return;
    }

    // Create the Resources tab widget
    resourcesTab_ = new QWidget(this);

    auto *mainLayout = new QVBoxLayout(resourcesTab_);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Top section: icon and device name
    auto *topLayout = new QHBoxLayout();
    auto *iconLabel = new QLabel(resourcesTab_);
    iconLabel->setFixedSize(32, 32);
    iconLabel->setPixmap(categoryIcon_.pixmap(32, 32));
    topLayout->addWidget(iconLabel);

    topLayout->addSpacing(15);

    auto *nameLabel = new QLabel(deviceInfo_->name(), resourcesTab_);
    topLayout->addWidget(nameLabel, 1);
    mainLayout->addLayout(topLayout);

    mainLayout->addSpacing(10);

    // Resource settings label
    auto *settingsLabel = new QLabel(tr("Resource settings:"), resourcesTab_);
    mainLayout->addWidget(settingsLabel);

    // Resource tree widget
    auto *resourceTree = new QTreeWidget(resourcesTab_);
    resourceTree->setHeaderLabels({tr("Resource type"), tr("Setting")});
    resourceTree->setRootIsDecorated(false);
    resourceTree->setAlternatingRowColors(true);

    for (const auto &res : resources) {
        auto *item = new QTreeWidgetItem(resourceTree);
        item->setIcon(0, QIcon::fromTheme(res.iconName));
        item->setText(0, res.type);
        item->setText(1, res.setting);
    }

    resourceTree->resizeColumnToContents(0);
    mainLayout->addWidget(resourceTree);

    mainLayout->addSpacing(10);

    // Settings based on (disabled)
    auto *settingsBasedLayout = new QHBoxLayout();
    auto *settingsBasedLabel = new QLabel(tr("Setting based on:"), resourcesTab_);
    settingsBasedLayout->addWidget(settingsBasedLabel);

    auto *settingsBasedCombo = new QComboBox(resourcesTab_);
    settingsBasedCombo->addItem(tr("Current configuration"));
    settingsBasedCombo->setEnabled(false);
    settingsBasedLayout->addWidget(settingsBasedCombo, 1);
    mainLayout->addLayout(settingsBasedLayout);

    mainLayout->addSpacing(5);

    // Use automatic settings checkbox and Change Setting button
    auto *autoSettingsLayout = new QHBoxLayout();
    auto *autoSettingsCheck = new QCheckBox(tr("Use automatic settings"), resourcesTab_);
    autoSettingsCheck->setChecked(true);
    autoSettingsCheck->setEnabled(false);
    autoSettingsLayout->addWidget(autoSettingsCheck);

    autoSettingsLayout->addStretch();

    auto *changeSettingBtn = new QPushButton(tr("Change Setting..."), resourcesTab_);
    changeSettingBtn->setEnabled(false);
    autoSettingsLayout->addWidget(changeSettingBtn);
    mainLayout->addLayout(autoSettingsLayout);

    mainLayout->addSpacing(10);

    // Conflicting device list group
    auto *conflictGroup = new QGroupBox(tr("Conflicting device list:"), resourcesTab_);
    auto *conflictLayout = new QVBoxLayout(conflictGroup);

    auto *conflictList = new QListWidget(conflictGroup);
    conflictList->addItem(tr("No conflicts."));
    conflictList->setEnabled(false);
    conflictLayout->addWidget(conflictList);

    mainLayout->addWidget(conflictGroup);

    // Add the Resources tab at the end
    tabWidget->addTab(resourcesTab_, tr("Resources"));
}
