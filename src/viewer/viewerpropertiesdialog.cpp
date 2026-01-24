#include "viewerpropertiesdialog.h"

#include "const_strings.h"
#include "viewerdriverdetailsdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

namespace s = strings;

ViewerPropertiesDialog::ViewerPropertiesDialog(QWidget *parent) : QDialog(parent) {
    setupUi();

    // Setup property mappings for Details tab
    propertyMappings_ = {
        {tr("Device description"), QStringLiteral("name")},
        {tr("Hardware IDs"), QStringLiteral("MODALIAS")},
        {tr("Device class"), QStringLiteral("ID_PCI_CLASS_FROM_DATABASE")},
        {tr("Driver key"), QStringLiteral("driver")},
        {tr("Device instance path"), QStringLiteral("devPath")},
        {tr("Physical device object name"), QStringLiteral("devnode")},
        {tr("Bus relations"), QStringLiteral("subsystem")},
        {tr("Parent"), QStringLiteral("parentSyspath")},
        {tr("Manufacturer"), QStringLiteral("ID_VENDOR_FROM_DATABASE")},
        {tr("Model"), QStringLiteral("ID_MODEL_FROM_DATABASE")},
        {tr("Serial number"), QStringLiteral("ID_SERIAL")},
        {tr("Subsystem"), QStringLiteral("subsystem")},
        {tr("Device node"), QStringLiteral("devnode")},
        {tr("Syspath"), QStringLiteral("syspath")},
    };

    connect(comboBoxDetailsProperty_,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &ViewerPropertiesDialog::onPropertySelectionChanged);
}

void ViewerPropertiesDialog::setupUi() {
    setMinimumSize(493, 502);
    setWindowTitle(tr("Device Properties"));

    auto *mainLayout = new QVBoxLayout(this);

    tabWidget_ = new QTabWidget(this);
    mainLayout->addWidget(tabWidget_);

    // === General Tab ===
    auto *tabGeneral = new QWidget();
    tabWidget_->addTab(tabGeneral, tr("General"));

    auto *generalLayout = new QVBoxLayout(tabGeneral);
    generalLayout->setContentsMargins(10, 10, 10, 10);

    // Header with icon and name
    auto *headerLayoutGeneral = new QHBoxLayout();
    labelIconGeneral_ = new QLabel();
    labelIconGeneral_->setFixedSize(32, 32);
    headerLayoutGeneral->addWidget(labelIconGeneral_);
    headerLayoutGeneral->addSpacing(15);
    labelDeviceNameGeneral_ = new QLabel();
    labelDeviceNameGeneral_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayoutGeneral->addWidget(labelDeviceNameGeneral_, 1);
    generalLayout->addLayout(headerLayoutGeneral);

    generalLayout->addSpacing(20);

    // Device info grid
    auto *infoGrid = new QGridLayout();
    infoGrid->setColumnStretch(2, 1);

    infoGrid->addWidget(new QLabel(tr("Device type:")), 0, 0);
    labelDeviceTypeValue_ = new QLabel(tr("Unknown"));
    labelDeviceTypeValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    infoGrid->addWidget(labelDeviceTypeValue_, 0, 2);

    infoGrid->addWidget(new QLabel(tr("Manufacturer:")), 1, 0);
    labelManufacturerValue_ = new QLabel(tr("Unknown"));
    labelManufacturerValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    infoGrid->addWidget(labelManufacturerValue_, 1, 2);

    infoGrid->addWidget(new QLabel(tr("Location:")), 2, 0);
    labelLocationValue_ = new QLabel(tr("Unknown"));
    labelLocationValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    infoGrid->addWidget(labelLocationValue_, 2, 2);

    generalLayout->addLayout(infoGrid);

    generalLayout->addSpacing(20);

    // Device status group
    auto *statusGroup = new QGroupBox(tr("Device status"));
    auto *statusLayout = new QVBoxLayout(statusGroup);
    textEditDeviceStatus_ = new QPlainTextEdit();
    textEditDeviceStatus_->setReadOnly(true);
    textEditDeviceStatus_->setMaximumHeight(100);
    statusLayout->addWidget(textEditDeviceStatus_);
    generalLayout->addWidget(statusGroup);

    generalLayout->addStretch();

    // === Driver Tab ===
    auto *tabDriver = new QWidget();
    tabWidget_->addTab(tabDriver, tr("Driver"));

    auto *driverLayout = new QVBoxLayout(tabDriver);
    driverLayout->setContentsMargins(10, 10, 10, 10);

    // Header with icon and name
    auto *headerLayoutDriver = new QHBoxLayout();
    labelIconDriver_ = new QLabel();
    labelIconDriver_->setFixedSize(32, 32);
    headerLayoutDriver->addWidget(labelIconDriver_);
    headerLayoutDriver->addSpacing(15);
    labelDeviceNameDriver_ = new QLabel();
    labelDeviceNameDriver_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayoutDriver->addWidget(labelDeviceNameDriver_, 1);
    driverLayout->addLayout(headerLayoutDriver);

    driverLayout->addSpacing(20);

    // Driver info grid
    auto *driverGrid = new QGridLayout();
    driverGrid->setColumnStretch(2, 1);

    driverGrid->addWidget(new QLabel(tr("Driver Provider:")), 0, 0);
    labelDriverProviderValue_ = new QLabel(tr("Unknown"));
    labelDriverProviderValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    driverGrid->addWidget(labelDriverProviderValue_, 0, 2);

    driverGrid->addWidget(new QLabel(tr("Driver Date:")), 1, 0);
    labelDriverDateValue_ = new QLabel(tr("Unknown"));
    labelDriverDateValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    driverGrid->addWidget(labelDriverDateValue_, 1, 2);

    driverGrid->addWidget(new QLabel(tr("Driver Version:")), 2, 0);
    labelDriverVersionValue_ = new QLabel(tr("Unknown"));
    labelDriverVersionValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    driverGrid->addWidget(labelDriverVersionValue_, 2, 2);

    driverGrid->addWidget(new QLabel(tr("Digital Signer:")), 3, 0);
    labelDigitalSignerValue_ = new QLabel(tr("Unknown"));
    labelDigitalSignerValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    driverGrid->addWidget(labelDigitalSignerValue_, 3, 2);

    driverLayout->addLayout(driverGrid);

    driverLayout->addSpacing(20);

    // Driver Details button
    auto *buttonGrid = new QGridLayout();
    buttonDriverDetails_ = new QPushButton(tr("Driver Details"));
    buttonDriverDetails_->setEnabled(false);
    connect(buttonDriverDetails_, &QPushButton::clicked, this, &ViewerPropertiesDialog::onDriverDetailsClicked);
    buttonGrid->addWidget(buttonDriverDetails_, 0, 0);
    buttonGrid->addWidget(new QLabel(tr("View details about the installed driver files.")), 0, 1);

    driverLayout->addLayout(buttonGrid);
    driverLayout->addStretch();

    // === Details Tab ===
    auto *tabDetails = new QWidget();
    tabWidget_->addTab(tabDetails, tr("Details"));

    auto *detailsLayout = new QVBoxLayout(tabDetails);
    detailsLayout->setContentsMargins(10, 10, 10, 10);

    // Header with icon and name
    auto *headerLayoutDetails = new QHBoxLayout();
    labelIconDetails_ = new QLabel();
    labelIconDetails_->setFixedSize(32, 32);
    headerLayoutDetails->addWidget(labelIconDetails_);
    headerLayoutDetails->addSpacing(15);
    labelDeviceNameDetails_ = new QLabel();
    labelDeviceNameDetails_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    headerLayoutDetails->addWidget(labelDeviceNameDetails_, 1);
    detailsLayout->addLayout(headerLayoutDetails);

    detailsLayout->addSpacing(10);

    detailsLayout->addWidget(new QLabel(tr("Property")));
    comboBoxDetailsProperty_ = new QComboBox();
    detailsLayout->addWidget(comboBoxDetailsProperty_);

    detailsLayout->addWidget(new QLabel(tr("Value")));
    listWidgetDetailsPropertyValue_ = new QListWidget();
    detailsLayout->addWidget(listWidgetDetailsPropertyValue_);

    // Button box
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void ViewerPropertiesDialog::setDeviceSyspath(const QString &syspath) {
    syspath_ = syspath;
    device_ = ExportedData::instance().deviceBySyspath(syspath);

    if (!device_) {
        return;
    }

    // Get display name
    QString deviceName = device_->name;
    if (device_->subsystem == QStringLiteral("acpi")) {
        deviceName = s::acpiDeviceNiceName(device_->devPath, deviceName);
    } else {
        deviceName = s::softwareDeviceNiceName(deviceName);
    }

    // Set device name on all tabs
    labelDeviceNameGeneral_->setText(deviceName);
    labelDeviceNameDriver_->setText(deviceName);
    labelDeviceNameDetails_->setText(deviceName);

    // Set window title
    setWindowTitle(tr("%1 Properties").arg(deviceName));

    populateGeneralTab();
    populateDriverTab();
    populateDetailsTab();
}

void ViewerPropertiesDialog::setCategoryIcon(const QIcon &icon) {
    categoryIcon_ = icon;
    QPixmap pixmap = icon.pixmap(32, 32);

    labelIconGeneral_->setPixmap(pixmap);
    labelIconDriver_->setPixmap(pixmap);
    labelIconDetails_->setPixmap(pixmap);
}

QString ViewerPropertiesDialog::getCategoryName(int category) {
    switch (category) {
    case 1:
        return tr("Audio inputs and outputs");
    case 2:
        return tr("Batteries");
    case 3:
        return tr("Computer");
    case 4:
        return tr("Disk drives");
    case 5:
        return tr("Display adapters");
    case 6:
        return tr("DVD/CD-ROM drives");
    case 7:
        return tr("Human Interface Devices");
    case 8:
        return tr("Keyboards");
    case 9:
        return tr("Mice and other pointing devices");
    case 10:
        return tr("Network adapters");
    case 11:
        return tr("Software devices");
    case 12:
        return tr("Sound, video and game controllers");
    case 13:
        return tr("Storage controllers");
    case 14:
        return tr("Storage volumes");
    case 15:
        return tr("System devices");
    case 16:
        return tr("Universal Serial Bus controllers");
    default:
        return device_ ? device_->subsystem : tr("Unknown");
    }
}

void ViewerPropertiesDialog::populateGeneralTab() {
    if (!device_) {
        return;
    }

    // Device type
    QString categoryName = device_->categoryName;
    if (categoryName.isEmpty()) {
        categoryName = getCategoryName(device_->category);
    }
    labelDeviceTypeValue_->setText(categoryName);

    // Manufacturer - try multiple sources from properties
    QString manufacturer =
        device_->properties[QStringLiteral("ID_VENDOR_FROM_DATABASE")].toString();
    if (manufacturer.isEmpty()) {
        manufacturer = device_->properties[QStringLiteral("ID_USB_VENDOR")].toString();
    }
    if (manufacturer.isEmpty()) {
        manufacturer = device_->properties[QStringLiteral("ID_VENDOR")].toString();
    }
    if (!manufacturer.isEmpty()) {
        labelManufacturerValue_->setText(manufacturer);
    }

    // Location - translate devpath
    QString devpath = device_->devPath;
    QString location = tr("Unknown");

    if (devpath.contains(QStringLiteral("/virtual/"))) {
        location = tr("Virtual device");
    } else if (devpath.contains(QStringLiteral("/usb"))) {
        location = tr("On USB bus");
    } else if (devpath.contains(QStringLiteral("/pci"))) {
        location = tr("On PCI bus");
    } else if (devpath.contains(QStringLiteral("/acpi")) || devpath.contains(QStringLiteral("/ACPI"))) {
        location = tr("On ACPI-compliant system");
    } else if (devpath.contains(QStringLiteral("/platform/"))) {
        location = tr("On system board");
    } else if (devpath.contains(QStringLiteral("/i2c"))) {
        location = tr("On I2C bus");
    }

    labelLocationValue_->setText(location);
    labelLocationValue_->setToolTip(devpath);

    // Device status
    if (!device_->driver.isEmpty()) {
        textEditDeviceStatus_->setPlainText(tr("This device is working properly."));
    } else {
        textEditDeviceStatus_->setPlainText(tr("No driver installed for this device."));
    }
}

void ViewerPropertiesDialog::populateDriverTab() {
    if (!device_) {
        return;
    }

    const QJsonObject &driverInfo = device_->driverInfo;

    // Driver provider
    QString provider = driverInfo[QStringLiteral("provider")].toString();
    if (provider.isEmpty()) {
        provider = device_->driver.isEmpty() ? tr("(No driver)") : tr("Linux Foundation");
    }
    labelDriverProviderValue_->setText(provider);

    // Driver version
    QString version = driverInfo[QStringLiteral("version")].toString();
    if (version.isEmpty()) {
        version = driverInfo[QStringLiteral("kernelVersion")].toString();
    }
    if (version.isEmpty()) {
        version = tr("N/A");
    }
    labelDriverVersionValue_->setText(version);

    // Driver date
    QString date = driverInfo[QStringLiteral("date")].toString();
    if (date.isEmpty()) {
        date = tr("N/A");
    }
    labelDriverDateValue_->setText(date);

    // Digital signer
    QString signer = driverInfo[QStringLiteral("signer")].toString();
    if (signer.isEmpty()) {
        signer = provider;
    }
    labelDigitalSignerValue_->setText(signer);

    // Enable Driver Details button if we have driver info
    bool hasDriverInfo = !device_->driver.isEmpty() && !driverInfo.isEmpty();
    buttonDriverDetails_->setEnabled(hasDriverInfo);
}

void ViewerPropertiesDialog::onDriverDetailsClicked() {
    if (!device_) {
        return;
    }

    ViewerDriverDetailsDialog dialog(this);
    dialog.setCategoryIcon(categoryIcon_);
    dialog.setDriverName(device_->driver);
    dialog.setDriverInfo(device_->driverInfo);
    dialog.exec();
}

void ViewerPropertiesDialog::populateDetailsTab() {
    if (!device_) {
        return;
    }

    comboBoxDetailsProperty_->clear();

    for (const PropertyMapping &mapping : propertyMappings_) {
        QString value;

        // Check direct device fields first
        if (mapping.jsonKey == QStringLiteral("name")) {
            value = device_->name;
        } else if (mapping.jsonKey == QStringLiteral("syspath")) {
            value = device_->syspath;
        } else if (mapping.jsonKey == QStringLiteral("parentSyspath")) {
            value = device_->parentSyspath;
        } else if (mapping.jsonKey == QStringLiteral("driver")) {
            value = device_->driver;
        } else if (mapping.jsonKey == QStringLiteral("subsystem")) {
            value = device_->subsystem;
        } else if (mapping.jsonKey == QStringLiteral("devnode")) {
            value = device_->devnode;
        } else if (mapping.jsonKey == QStringLiteral("devPath")) {
            value = device_->devPath;
        } else {
            // Check properties object
            value = device_->properties[mapping.jsonKey].toString();
        }

        if (!value.isEmpty()) {
            comboBoxDetailsProperty_->addItem(mapping.displayName, mapping.jsonKey);
        }
    }

    // Trigger initial selection
    if (comboBoxDetailsProperty_->count() > 0) {
        onPropertySelectionChanged(0);
    }
}

void ViewerPropertiesDialog::onPropertySelectionChanged(int index) {
    listWidgetDetailsPropertyValue_->clear();

    if (index < 0 || !device_) {
        return;
    }

    QString jsonKey = comboBoxDetailsProperty_->currentData().toString();
    QString value;

    // Check direct device fields first
    if (jsonKey == QStringLiteral("name")) {
        value = device_->name;
    } else if (jsonKey == QStringLiteral("syspath")) {
        value = device_->syspath;
    } else if (jsonKey == QStringLiteral("parentSyspath")) {
        value = device_->parentSyspath;
    } else if (jsonKey == QStringLiteral("driver")) {
        value = device_->driver;
    } else if (jsonKey == QStringLiteral("subsystem")) {
        value = device_->subsystem;
    } else if (jsonKey == QStringLiteral("devnode")) {
        value = device_->devnode;
    } else if (jsonKey == QStringLiteral("devPath")) {
        value = device_->devPath;
    } else {
        value = device_->properties[jsonKey].toString();
    }

    if (!value.isEmpty()) {
        if (value.contains(QStringLiteral("\n"))) {
            QStringList values = value.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
            for (const QString &v : values) {
                listWidgetDetailsPropertyValue_->addItem(v.trimmed());
            }
        } else if (value.contains(QStringLiteral(";"))) {
            QStringList values = value.split(QStringLiteral(";"), Qt::SkipEmptyParts);
            for (const QString &v : values) {
                listWidgetDetailsPropertyValue_->addItem(v.trimmed());
            }
        } else {
            listWidgetDetailsPropertyValue_->addItem(value);
        }
    }
}
