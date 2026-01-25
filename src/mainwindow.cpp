#include <QFileDialog>
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QStandardPaths>
#include <QUrl>
#include <QWhatsThis>
#include <QtConcurrent>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QDesktopServices>

#ifdef DEVMGMT_USE_KDE
#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KActionCollection>
#include <KHelpMenu>
#include <KLocalizedString>
#include <KShortcutsDialog>
#include <KStandardAction>
#endif

#include "customizedialog.h"
#include "devicecache.h"
#include "systeminfo.h"
#include "deviceexport.h"
#include "mainwindow.h"
#include "models/devbyconnmodel.h"
#include "models/devbydrivermodel.h"
#include "models/devbytypemodel.h"
#include "models/drvbydevmodel.h"
#include "models/drvbytypemodel.h"
#include "models/node.h"
#include "models/resbyconnmodel.h"
#include "models/resbytypemodel.h"
#include "propertiesdialog.h"
#include "viewsettings.h"

MainWindow::MainWindow() {
    setupUi(this);
    setUnifiedTitleAndToolBarOnMac(true);

    actionGroupView = new QActionGroup(this);
    actionGroupView->addAction(actionDevicesByType);
    actionGroupView->addAction(actionDevicesByConnection);
    actionGroupView->addAction(actionDevicesByDriver);
    actionGroupView->addAction(actionDriversByType);
    actionGroupView->addAction(actionDriversByDevice);
    actionGroupView->addAction(actionResourcesByType);
    actionGroupView->addAction(actionResourcesByConnection);
    actionGroupView->setExclusive(true);

    connect(actionDevicesByType, &QAction::triggered, [this]() {
        currentViewAction = actionDevicesByType;
        ViewSettings::instance().setLastView(QStringLiteral("DevicesByType"));
        switchToDevicesByType();
    });
    connect(actionDevicesByConnection, &QAction::triggered, [this]() {
        currentViewAction = actionDevicesByConnection;
        ViewSettings::instance().setLastView(QStringLiteral("DevicesByConnection"));
        switchToModel(new DevicesByConnectionModel(this));
    });
    connect(actionDevicesByDriver, &QAction::triggered, [this]() {
        currentViewAction = actionDevicesByDriver;
        ViewSettings::instance().setLastView(QStringLiteral("DevicesByDriver"));
        switchToModel(new DevicesByDriverModel(this));
    });
    connect(actionDriversByType, &QAction::triggered, [this]() {
        currentViewAction = actionDriversByType;
        ViewSettings::instance().setLastView(QStringLiteral("DriversByType"));
        switchToModel(new DriversByTypeModel(this));
    });
    connect(actionDriversByDevice, &QAction::triggered, [this]() {
        currentViewAction = actionDriversByDevice;
        ViewSettings::instance().setLastView(QStringLiteral("DriversByDevice"));
        switchToModel(new DriversByDeviceModel(this));
    });
    connect(actionResourcesByType, &QAction::triggered, [this]() {
        currentViewAction = actionResourcesByType;
        ViewSettings::instance().setLastView(QStringLiteral("ResourcesByType"));
        switchToModel(new ResourcesByTypeModel(this));
    });
    connect(actionResourcesByConnection, &QAction::triggered, [this]() {
        currentViewAction = actionResourcesByConnection;
        ViewSettings::instance().setLastView(QStringLiteral("ResourcesByConnection"));
        switchToModel(new ResourcesByConnectionModel(this));
    });

    // Restore last view on startup
    restoreLastView();

    // Show hidden devices toggle - restore state from settings
    actionShowHiddenDevices->setChecked(ViewSettings::instance().showHiddenDevices());
    connect(actionShowHiddenDevices, &QAction::toggled, this, &MainWindow::toggleShowHiddenDevices);

    // Scan for hardware changes (F5)
    connect(actionScanForHardwareChanges,
            &QAction::triggered,
            this,
            &MainWindow::scanForHardwareChanges);

    // Apply initial view settings
    applyViewSettings();

    // Export action
    connect(actionExport, &QAction::triggered, this, &MainWindow::exportDeviceData);

    // Open action (for viewing exported files)
    connect(actionOpen, &QAction::triggered, this, &MainWindow::openExportFile);

    // Return to live view action
    connect(actionReturnToLive, &QAction::triggered, this, &MainWindow::returnToLiveView);

#ifdef DEVMGMT_USE_KDE
    // For KDE, register actions before setupGUI()
    setupActions();
#else
    setupMenus();
#endif

    // Devices and Printers - open platform-specific printers settings
    connect(actionDevicesAndPrinters, &QAction::triggered, []() { openPrintersSettings(); });

    // Open properties dialog on double-click (for devices)
    connect(treeView, &QTreeView::activated, this, &MainWindow::openPropertiesForIndex);

    // Install event filter to handle Enter key for expand/collapse on categories
    treeView->installEventFilter(this);

    // Connect to device monitor for automatic refresh on device changes
    connectDeviceMonitor();

#ifdef DEVMGMT_USE_KDE
    // Don't use setupGUI() as it replaces the menubar from .ui file
    // Instead, manually set up KDE-specific menus
    postSetupMenus();
#endif
}

void MainWindow::switchToModel(QAbstractItemModel *model, int depth) {
    auto *oldModel = treeView->model();
    treeView->setModel(model);
    if (oldModel && oldModel != model) {
        oldModel->deleteLater();
    }
    if (model != nullptr) {
        if (ViewSettings::instance().expandAllOnLoad()) {
            treeView->expandAll();
        } else {
            treeView->expandToDepth(depth);
        }
    }
    applyViewSettings();
}

void MainWindow::switchToDevicesByType() {
    auto *oldModel = treeView->model();
    auto *model = new DevicesByTypeModel(this);
    treeView->setModel(model);
    if (oldModel && oldModel != model) {
        oldModel->deleteLater();
    }
    if (ViewSettings::instance().expandAllOnLoad()) {
        treeView->expandAll();
    } else {
        // Expand only the hostname, keeping all categories collapsed
        treeView->expandToDepth(0);
    }
    applyViewSettings();
}

void MainWindow::restoreLastView() {
    auto lastView = ViewSettings::instance().lastView();

    if (lastView == QStringLiteral("DevicesByConnection")) {
        actionDevicesByConnection->setChecked(true);
        currentViewAction = actionDevicesByConnection;
        switchToModel(new DevicesByConnectionModel(this));
    } else if (lastView == QStringLiteral("DevicesByDriver")) {
        actionDevicesByDriver->setChecked(true);
        currentViewAction = actionDevicesByDriver;
        switchToModel(new DevicesByDriverModel(this));
    } else if (lastView == QStringLiteral("DriversByType")) {
        actionDriversByType->setChecked(true);
        currentViewAction = actionDriversByType;
        switchToModel(new DriversByTypeModel(this));
    } else if (lastView == QStringLiteral("DriversByDevice")) {
        actionDriversByDevice->setChecked(true);
        currentViewAction = actionDriversByDevice;
        switchToModel(new DriversByDeviceModel(this));
    } else if (lastView == QStringLiteral("ResourcesByType")) {
        actionResourcesByType->setChecked(true);
        currentViewAction = actionResourcesByType;
        switchToModel(new ResourcesByTypeModel(this));
    } else if (lastView == QStringLiteral("ResourcesByConnection")) {
        actionResourcesByConnection->setChecked(true);
        currentViewAction = actionResourcesByConnection;
        switchToModel(new ResourcesByConnectionModel(this));
    } else {
        // Default: DevicesByType
        actionDevicesByType->setChecked(true);
        currentViewAction = actionDevicesByType;
        switchToDevicesByType();
    }
}

void MainWindow::about() {
#ifdef DEVMGMT_USE_KDE
    KAboutApplicationDialog dialog(KAboutData::applicationData(), this);
    dialog.exec();
#else
    QMessageBox::about(this,
                       tr("About Device Manager"),
                       tr("View and manage device hardware settings and "
                          "driver software installed on your computer."));
#endif
}

void MainWindow::openPropertiesForIndex(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto *node = static_cast<Node *>(index.internalPointer());
    if (node->type() == NodeType::Device) {
        PropertiesDialog dialog(this);
        dialog.setCategoryIcon(node->icon());
        dialog.setDeviceSyspath(node->syspath());
        dialog.setWindowModality(Qt::WindowModal);
        dialog.exec();
    }
    // Category nodes: double-click expand/collapse is handled by QTreeView automatically
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == treeView && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QModelIndex index = treeView->currentIndex();
            if (index.isValid()) {
                auto *node = static_cast<Node *>(index.internalPointer());
                if (node->type() == NodeType::Device) {
                    openPropertiesForIndex(index);
                    return true;
                } else {
                    // Toggle expand/collapse for category nodes
                    if (treeView->isExpanded(index)) {
                        treeView->collapse(index);
                    } else {
                        treeView->expand(index);
                    }
                    return true;
                }
            }
        }
    }
#ifdef DEVMGMT_USE_KDE
    return KXmlGuiWindow::eventFilter(watched, event);
#else
    return QMainWindow::eventFilter(watched, event);
#endif
}

void MainWindow::toggleShowHiddenDevices(bool checked) {
    DeviceCache::instance().setShowHiddenDevices(checked);
    refreshCurrentView();
}

void MainWindow::refreshCurrentView() {
    // Save expanded state before rebuilding
    auto expandedPaths = saveExpandedState();

    // Trigger the current view action to rebuild the model
    if (currentViewAction == actionDevicesByType) {
        auto *oldModel = treeView->model();
        auto *model = new DevicesByTypeModel(this);
        treeView->setModel(model);
        if (oldModel && oldModel != model) {
            oldModel->deleteLater();
        }
    } else if (currentViewAction == actionDevicesByConnection) {
        auto *oldModel = treeView->model();
        treeView->setModel(new DevicesByConnectionModel(this));
        if (oldModel) {
            oldModel->deleteLater();
        }
    } else if (currentViewAction == actionDevicesByDriver) {
        auto *oldModel = treeView->model();
        treeView->setModel(new DevicesByDriverModel(this));
        if (oldModel) {
            oldModel->deleteLater();
        }
    } else if (currentViewAction == actionDriversByType) {
        auto *oldModel = treeView->model();
        treeView->setModel(new DriversByTypeModel(this));
        if (oldModel) {
            oldModel->deleteLater();
        }
    } else if (currentViewAction == actionDriversByDevice) {
        auto *oldModel = treeView->model();
        treeView->setModel(new DriversByDeviceModel(this));
        if (oldModel) {
            oldModel->deleteLater();
        }
    } else if (currentViewAction == actionResourcesByType) {
        auto *oldModel = treeView->model();
        treeView->setModel(new ResourcesByTypeModel(this));
        if (oldModel) {
            oldModel->deleteLater();
        }
    } else if (currentViewAction == actionResourcesByConnection) {
        auto *oldModel = treeView->model();
        treeView->setModel(new ResourcesByConnectionModel(this));
        if (oldModel) {
            oldModel->deleteLater();
        }
    }

    // Restore expanded state
    restoreExpandedState(expandedPaths);
    applyViewSettings();
}

void MainWindow::scanForHardwareChanges() {
    // If a scan is already in progress, don't start another
    if (scanWatcher_ && scanWatcher_->isRunning()) {
        return;
    }

    // Create a modal progress dialog that cannot be cancelled
    scanProgressDialog =
        new QProgressDialog(tr("Scanning for hardware changes..."), QString(), 0, 0, this);
    scanProgressDialog->setWindowTitle(tr("Device Manager"));
    scanProgressDialog->setWindowModality(Qt::WindowModal);
    scanProgressDialog->setCancelButton(nullptr); // No cancel button
    scanProgressDialog->setMinimumDuration(0);    // Show immediately
    scanProgressDialog->show();

    // Clean up previous watcher if any
    if (scanWatcher_) {
        scanWatcher_->disconnect();
        delete scanWatcher_;
    }

    // Run the refresh in a background thread
    scanWatcher_ = new QFutureWatcher<void>(this);
    connect(scanWatcher_, &QFutureWatcher<void>::finished, this, &MainWindow::onScanComplete);

    auto future = QtConcurrent::run([]() { DeviceCache::instance().refresh(); });
    scanWatcher_->setFuture(future);
}

void MainWindow::onScanComplete() {
    // Close the progress dialog
    if (scanProgressDialog) {
        scanProgressDialog->close();
        scanProgressDialog->deleteLater();
        scanProgressDialog = nullptr;
    }

    // Clean up the watcher
    if (scanWatcher_) {
        scanWatcher_->deleteLater();
        scanWatcher_ = nullptr;
    }

    // Refresh the current view
    refreshCurrentView();
}

MainWindow::~MainWindow() {
    // Wait for any pending scan operation to complete
    if (scanWatcher_) {
        scanWatcher_->disconnect();
        scanWatcher_->waitForFinished();
        delete scanWatcher_;
        scanWatcher_ = nullptr;
    }
}

void MainWindow::showCustomizeDialog() {
    CustomizeDialog dialog(this);

    // Load current settings into dialog
    auto &settings = ViewSettings::instance();
    dialog.setShowDeviceIcons(settings.showDeviceIcons());
    dialog.setExpandAllOnLoad(settings.expandAllOnLoad());
    dialog.setShowDriverColumn(settings.showDriverColumn());

    if (dialog.exec() == QDialog::Accepted) {
        // Save settings
        settings.setShowDeviceIcons(dialog.showDeviceIcons());
        settings.setExpandAllOnLoad(dialog.expandAllOnLoad());
        settings.setShowDriverColumn(dialog.showDriverColumn());
        settings.save();

        // Apply settings
        applyViewSettings();

        // Refresh view to apply changes
        refreshCurrentView();
    }
}

void MainWindow::exportDeviceData() {
    // Generate default filename with hostname and timestamp
    QString defaultName =
        QStringLiteral("%1_%2%3")
            .arg(DeviceCache::hostname())
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd_HHmmss")))
            .arg(QLatin1String(DeviceExport::FILE_EXTENSION));

    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString defaultPath = defaultDir + QLatin1Char('/') + defaultName;

    QString filePath =
        QFileDialog::getSaveFileName(this,
                                     tr("Export Device Data"),
                                     defaultPath,
                                     tr("Device Manager Export (*%1);;All Files (*)")
                                         .arg(QLatin1String(DeviceExport::FILE_EXTENSION)));

    if (filePath.isEmpty()) {
        return;
    }

    // Ensure the file has the correct extension
    if (!filePath.endsWith(QLatin1String(DeviceExport::FILE_EXTENSION), Qt::CaseInsensitive)) {
        filePath += QLatin1String(DeviceExport::FILE_EXTENSION);
    }

    // Show progress dialog
    auto *progressDialog =
        new QProgressDialog(tr("Exporting device data..."), QString(), 0, 0, this);
    progressDialog->setWindowTitle(tr("Device Manager"));
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimumDuration(0);
    progressDialog->show();

    // Run export in background thread
    auto *watcher = new QFutureWatcher<bool>(this);
    connect(watcher,
            &QFutureWatcher<bool>::finished,
            this,
            [this, watcher, progressDialog, filePath]() {
                progressDialog->close();
                progressDialog->deleteLater();

                bool success = watcher->result();
                watcher->deleteLater();

                if (!success) {
                    QMessageBox::warning(this,
                                         tr("Export Failed"),
                                         tr("Failed to export device data to:\n%1").arg(filePath));
                }
            });

    // Capture device data before starting the background thread
    QList<DeviceInfo> devices = DeviceCache::instance().allDevices();
    QString hostname = DeviceCache::hostname();

    auto future = QtConcurrent::run([filePath, devices, hostname]() {
        return DeviceExport::exportToFile(filePath, devices, hostname);
    });
    watcher->setFuture(future);
}

void MainWindow::openExportFile() {
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filePath =
        QFileDialog::getOpenFileName(this,
                                     tr("Open Device Manager Export"),
                                     defaultDir,
                                     tr("Device Manager Export (*%1);;All Files (*)")
                                         .arg(QLatin1String(DeviceExport::FILE_EXTENSION)));

    if (filePath.isEmpty()) {
        return;
    }

    if (!loadExportFile(filePath)) {
        QMessageBox::warning(
            this, tr("Error"), tr("Failed to load export file:\n%1").arg(filePath));
    }
}

bool MainWindow::loadExportFile(const QString &filePath) {
    if (!DeviceCache::instance().loadFromFile(filePath)) {
        return false;
    }

    // Update UI for viewer mode
    actionReturnToLive->setEnabled(true);
    actionExport->setEnabled(false);
    actionScanForHardwareChanges->setEnabled(false);
#ifdef DEVMGMT_USE_KDE
    if (auto *refreshAction = actionCollection()->action(QStringLiteral("view_refresh"))) {
        refreshAction->setEnabled(false);
    }
    if (auto *exportAction = actionCollection()->action(QStringLiteral("file_export"))) {
        exportAction->setEnabled(false);
    }
#endif
    if (menuAction) {
        menuAction->setEnabled(false);
    }

    // Update window title
    QFileInfo fi(filePath);
#ifdef DEVMGMT_USE_KDE
    // KDE appends " -- <app name>" automatically, so just set the document part
    setWindowTitle(tr("%1 - %2").arg(fi.fileName(), DeviceCache::hostname()));
#else
    setWindowTitle(tr("%1 - %2 —  Device Manager").arg(fi.fileName(), DeviceCache::hostname()));
#endif

    // Refresh the current view
    refreshCurrentView();
    return true;
}

void MainWindow::returnToLiveView() {
    DeviceCache::instance().reloadLiveData();

    // Update UI for live mode
    actionReturnToLive->setEnabled(false);
    actionExport->setEnabled(true);
    actionScanForHardwareChanges->setEnabled(true);
#ifdef DEVMGMT_USE_KDE
    if (auto *refreshAction = actionCollection()->action(QStringLiteral("view_refresh"))) {
        refreshAction->setEnabled(true);
    }
    if (auto *exportAction = actionCollection()->action(QStringLiteral("file_export"))) {
        exportAction->setEnabled(true);
    }
#endif
    if (menuAction) {
        menuAction->setEnabled(true);
    }

    // Restore window title
#ifdef DEVMGMT_USE_KDE
    // KDE appends " -- <app name>" automatically, so clear the document part
    setWindowTitle(QString());
#else
    setWindowTitle(tr("Device Manager"));
#endif

    // Refresh the current view
    refreshCurrentView();
}

void MainWindow::applyViewSettings() {
    auto &settings = ViewSettings::instance();

    // Driver column only applies to "Devices by type" view - other views only have 1 column
    bool showDriverColumn = settings.showDriverColumn() && currentViewAction == actionDevicesByType;

    // Show/hide driver column (column 1)
    if (showDriverColumn) {
        treeView->header()->setVisible(true);
        treeView->setHeaderHidden(false);
        // Name column interactive, Driver column with fixed width
        treeView->header()->setStretchLastSection(false);
        treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        treeView->header()->setSectionResizeMode(1, QHeaderView::Interactive);
        treeView->header()->setMinimumSectionSize(100);
        treeView->setColumnWidth(1, 150);
    } else {
        treeView->header()->setVisible(false);
        treeView->setHeaderHidden(true);
        // Single column mode - stretch to fill 100% of parent width
        treeView->header()->setStretchLastSection(false);
        treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        // Hide any additional columns that may exist
        for (int i = 1; i < treeView->header()->count(); ++i) {
            treeView->setColumnHidden(i, true);
        }
    }
}

void MainWindow::connectDeviceMonitor() {
    connect(&DeviceCache::instance(),
            &DeviceCache::devicesChanged,
            this,
            &MainWindow::refreshCurrentView);
}

QSet<QString> MainWindow::saveExpandedState() const {
    QSet<QString> expandedPaths;
    auto *model = treeView->model();
    if (!model) {
        return expandedPaths;
    }
    collectExpandedPaths(QModelIndex(), QString(), expandedPaths);
    return expandedPaths;
}

void MainWindow::collectExpandedPaths(const QModelIndex &parent,
                                      const QString &parentPath,
                                      QSet<QString> &expandedPaths) const {
    auto *model = treeView->model();
    if (!model) {
        return;
    }

    int rowCount = model->rowCount(parent);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0, parent);
        if (!index.isValid()) {
            continue;
        }

        // Build the path using the display name
        QString name = model->data(index, Qt::DisplayRole).toString();
        QString path = parentPath.isEmpty() ? name : parentPath + QStringLiteral("/") + name;

        if (treeView->isExpanded(index)) {
            expandedPaths.insert(path);
            // Recursively check children
            collectExpandedPaths(index, path, expandedPaths);
        }
    }
}

void MainWindow::restoreExpandedState(const QSet<QString> &expandedPaths) {
    if (expandedPaths.isEmpty()) {
        return;
    }
    expandMatchingPaths(QModelIndex(), QString(), expandedPaths);
}

void MainWindow::expandMatchingPaths(const QModelIndex &parent,
                                     const QString &parentPath,
                                     const QSet<QString> &expandedPaths) {
    auto *model = treeView->model();
    if (!model) {
        return;
    }

    int rowCount = model->rowCount(parent);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0, parent);
        if (!index.isValid()) {
            continue;
        }

        // Build the path using the display name
        QString name = model->data(index, Qt::DisplayRole).toString();
        QString path = parentPath.isEmpty() ? name : parentPath + QStringLiteral("/") + name;

        if (expandedPaths.contains(path)) {
            treeView->expand(index);
            // Recursively expand children
            expandMatchingPaths(index, path, expandedPaths);
        }
    }
}

#ifdef DEVMGMT_USE_KDE
void MainWindow::setupActions() {
    // For KDE, add actions to the action collection for shortcut management

    // Refresh action (F5) - use KStandardAction for proper KDE integration
    auto *refreshAction =
        KStandardAction::redisplay(this, &MainWindow::scanForHardwareChanges, this);
    refreshAction->setText(i18n("&Refresh"));
    actionCollection()->addAction(QStringLiteral("view_refresh"), refreshAction);

    // Clear the F5 shortcut from the original action to avoid conflict
    actionScanForHardwareChanges->setShortcut(QKeySequence());

    // Create Configure Device Manager action for Settings menu
    auto *configureAction =
        KStandardAction::preferences(this, &MainWindow::showCustomizeDialog, this);
    configureAction->setText(i18n("Configure Device Manager..."));
    actionCollection()->addAction(KStandardAction::name(KStandardAction::Preferences),
                                  configureAction);
}

void MainWindow::postSetupMenus() {
    // Rebuild File menu for KDE HIG
    if (menuFile) {
        menuFile->clear();

        // Open action
        auto *openAction =
            new QAction(QIcon::fromTheme(QStringLiteral("document-open")), i18n("&Open..."), this);
        openAction->setShortcut(QKeySequence::Open);
        connect(openAction, &QAction::triggered, this, &MainWindow::openExportFile);
        menuFile->addAction(openAction);

        // Export action
        auto *exportAction = new QAction(
            QIcon::fromTheme(QStringLiteral("document-export")), i18n("&Export..."), this);
        exportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
        connect(exportAction, &QAction::triggered, this, &MainWindow::exportDeviceData);
        actionCollection()->addAction(QStringLiteral("file_export"), exportAction);
        menuFile->addAction(exportAction);

        menuFile->addSeparator();

        // Return to Live View action
        auto *returnToLiveAction = new QAction(
            QIcon::fromTheme(QStringLiteral("view-refresh")), i18n("&Return to Live View"), this);
        returnToLiveAction->setEnabled(false);
        connect(returnToLiveAction, &QAction::triggered, this, &MainWindow::returnToLiveView);
        menuFile->addAction(returnToLiveAction);
        // Store reference for later enable/disable
        actionReturnToLive = returnToLiveAction;

        menuFile->addSeparator();

        // Quit action
        auto *quitAction = KStandardAction::quit(this, &MainWindow::close, this);
        actionCollection()->addAction(KStandardAction::name(KStandardAction::Quit), quitAction);
        menuFile->addAction(quitAction);
    }

    // Remove Help from Action menu (KDE HIG - help items go in Help menu)
    if (menuAction) {
        menuAction->removeAction(actionHelp);
    }

    // Modify View menu: add Refresh at top, remove Customize
    if (menuView) {
        // Add Refresh action at the beginning of View menu
        auto *refreshAction = actionCollection()->action(QStringLiteral("view_refresh"));
        if (refreshAction) {
            auto firstAction =
                menuView->actions().isEmpty() ? nullptr : menuView->actions().first();
            menuView->insertAction(firstAction, refreshAction);
            menuView->insertSeparator(firstAction);
        }

        // Remove Customize (now in Settings menu as Configure)
        menuView->removeAction(actionCustomize);
    }

    // Create Settings menu (KDE HIG)
    auto *settingsMenu = new QMenu(i18n("&Settings"), this);

    // Configure Shortcuts...
    auto *configureShortcutsAction =
        new QAction(QIcon::fromTheme(QStringLiteral("configure-shortcuts")),
                    i18n("Configure Keyboard Shortcuts..."),
                    this);
    connect(configureShortcutsAction, &QAction::triggered, this, [this]() {
        KShortcutsDialog::showDialog(
            actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this);
    });
    settingsMenu->addAction(configureShortcutsAction);

    // Configure Device Manager...
    auto *configureAction =
        actionCollection()->action(KStandardAction::name(KStandardAction::Preferences));
    if (configureAction) {
        settingsMenu->addAction(configureAction);
    }

    // Insert Settings menu before Help menu
    menubar->insertMenu(menuHelp->menuAction(), settingsMenu);

    // Rebuild Help menu (KDE HIG)
    menuHelp->clear();

    // Handbook
    auto *handbookAction = new QAction(
        QIcon::fromTheme(QStringLiteral("help-contents")), i18n("Device Manager &Handbook"), this);
    connect(handbookAction, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral(DEVMGMT_WEBSITE_URL "/wiki")));
    });
    menuHelp->addAction(handbookAction);

    // What's This?
    auto *whatsThisAction = new QAction(
        QIcon::fromTheme(QStringLiteral("help-whatsthis")), i18n("What's &This?"), this);
    whatsThisAction->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_F1));
    connect(whatsThisAction, &QAction::triggered, []() { QWhatsThis::enterWhatsThisMode(); });
    menuHelp->addAction(whatsThisAction);

    menuHelp->addSeparator();

    // Report Bug...
    auto *reportBugAction = new QAction(
        QIcon::fromTheme(QStringLiteral("tools-report-bug")), i18n("&Report Bug..."), this);
    connect(reportBugAction, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral(DEVMGMT_WEBSITE_URL "/issues")));
    });
    menuHelp->addAction(reportBugAction);

    menuHelp->addSeparator();

    // About Device Manager
    auto *aboutAppAction = new QAction(
        QIcon::fromTheme(QStringLiteral("help-about")), i18n("&About Device Manager"), this);
    connect(aboutAppAction, &QAction::triggered, this, &MainWindow::about);
    menuHelp->addAction(aboutAppAction);

    // About KDE
    auto *kdeHelpMenu = new KHelpMenu(this, KAboutData::applicationData());
    menuHelp->addAction(kdeHelpMenu->action(KHelpMenu::menuAboutKDE));
}
#endif

void MainWindow::setupMenus() {
#ifndef DEVMGMT_USE_KDE
    // Non-KDE: Use standard Qt menus
    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);

    // Customize view settings (View -> Customize)
    connect(actionCustomize, &QAction::triggered, this, &MainWindow::showCustomizeDialog);

    // Website - open project website
    connect(actionWebsite, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral(DEVMGMT_WEBSITE_URL)));
    });

    // Exit action
    connect(actionExit, &QAction::triggered, this, &QMainWindow::close);

    // Remove Help from Action menu
    if (menuAction) {
        menuAction->removeAction(actionHelp);
    }

    // Remove Help Topics from Help menu
    if (menuHelp) {
        menuHelp->removeAction(actionHelpTopics);
    }
#endif
}
