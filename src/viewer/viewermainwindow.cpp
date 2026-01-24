#include "viewermainwindow.h"

#include "const_strings.h"
#include "deviceexport.h"
#include "exporteddata.h"
#include "models/node.h"
#include "viewerdevbyconnmodel.h"
#include "viewerdevbydrivermodel.h"
#include "viewerdevbytypemodel.h"
#include "viewerdrvbydevmodel.h"
#include "viewerdrvbytypemodel.h"
#include "viewerpropertiesdialog.h"
#include "viewerresbyconnmodel.h"
#include "viewerresbytypemodel.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>

ViewerMainWindow::ViewerMainWindow() {
    setupUi();
    setupMenus();

    // Install event filter for Enter key
    treeView_->installEventFilter(this);
}

void ViewerMainWindow::setupUi() {
    setWindowTitle(tr("Device Manager Viewer"));
    resize(765, 639);

    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    treeView_ = new QTreeView(centralWidget);
    treeView_->setAnimated(true);
    treeView_->setHeaderHidden(true);
    layout->addWidget(treeView_);

    // Connect double-click to properties
    connect(treeView_, &QTreeView::activated, this, &ViewerMainWindow::openPropertiesForIndex);
}

void ViewerMainWindow::setupMenus() {
    // File menu
    auto *menuFile = menuBar()->addMenu(tr("&File"));

    auto *actionOpen = new QAction(tr("&Open..."), this);
    actionOpen->setShortcut(QKeySequence::Open);
    connect(actionOpen, &QAction::triggered, this, &ViewerMainWindow::openNewFile);
    menuFile->addAction(actionOpen);

    menuFile->addSeparator();

    auto *actionExit = new QAction(tr("E&xit"), this);
    actionExit->setShortcut(QKeySequence::Quit);
    connect(actionExit, &QAction::triggered, this, &QMainWindow::close);
    menuFile->addAction(actionExit);

    // View menu
    auto *menuView = menuBar()->addMenu(tr("&View"));

    actionGroupView_ = new QActionGroup(this);

    actionDevicesByType_ = new QAction(tr("D&evices by type"), this);
    actionDevicesByType_->setCheckable(true);
    actionDevicesByType_->setChecked(true);
    actionGroupView_->addAction(actionDevicesByType_);
    menuView->addAction(actionDevicesByType_);

    actionDevicesByConnection_ = new QAction(tr("De&vices by connection"), this);
    actionDevicesByConnection_->setCheckable(true);
    actionGroupView_->addAction(actionDevicesByConnection_);
    menuView->addAction(actionDevicesByConnection_);

    actionDevicesByDriver_ = new QAction(tr("Dev&ices by driver"), this);
    actionDevicesByDriver_->setCheckable(true);
    actionGroupView_->addAction(actionDevicesByDriver_);
    menuView->addAction(actionDevicesByDriver_);

    actionDriversByType_ = new QAction(tr("Drive&rs by type"), this);
    actionDriversByType_->setCheckable(true);
    actionGroupView_->addAction(actionDriversByType_);
    menuView->addAction(actionDriversByType_);

    actionDriversByDevice_ = new QAction(tr("Driver&s by device"), this);
    actionDriversByDevice_->setCheckable(true);
    actionGroupView_->addAction(actionDriversByDevice_);
    menuView->addAction(actionDriversByDevice_);

    actionResourcesByType_ = new QAction(tr("Resources by t&ype"), this);
    actionResourcesByType_->setCheckable(true);
    actionGroupView_->addAction(actionResourcesByType_);
    menuView->addAction(actionResourcesByType_);

    actionResourcesByConnection_ = new QAction(tr("Resources by co&nnection"), this);
    actionResourcesByConnection_->setCheckable(true);
    actionGroupView_->addAction(actionResourcesByConnection_);
    menuView->addAction(actionResourcesByConnection_);

    actionGroupView_->setExclusive(true);

    // Connect view actions
    connect(actionDevicesByType_, &QAction::triggered, [this]() {
        currentViewAction_ = actionDevicesByType_;
        switchToDevicesByType();
    });
    connect(actionDevicesByConnection_, &QAction::triggered, [this]() {
        currentViewAction_ = actionDevicesByConnection_;
        switchToModel(new ViewerDevicesByConnectionModel(this));
    });
    connect(actionDevicesByDriver_, &QAction::triggered, [this]() {
        currentViewAction_ = actionDevicesByDriver_;
        switchToModel(new ViewerDevicesByDriverModel(this));
    });
    connect(actionDriversByType_, &QAction::triggered, [this]() {
        currentViewAction_ = actionDriversByType_;
        switchToModel(new ViewerDriversByTypeModel(this));
    });
    connect(actionDriversByDevice_, &QAction::triggered, [this]() {
        currentViewAction_ = actionDriversByDevice_;
        switchToModel(new ViewerDriversByDeviceModel(this));
    });
    connect(actionResourcesByType_, &QAction::triggered, [this]() {
        currentViewAction_ = actionResourcesByType_;
        switchToModel(new ViewerResourcesByTypeModel(this));
    });
    connect(actionResourcesByConnection_, &QAction::triggered, [this]() {
        currentViewAction_ = actionResourcesByConnection_;
        switchToModel(new ViewerResourcesByConnectionModel(this));
    });

    menuView->addSeparator();

    actionShowHiddenDevices_ = new QAction(tr("Sho&w hidden devices"), this);
    actionShowHiddenDevices_->setCheckable(true);
    actionShowHiddenDevices_->setChecked(true);
    connect(actionShowHiddenDevices_, &QAction::triggered, this, &ViewerMainWindow::toggleShowHiddenDevices);
    menuView->addAction(actionShowHiddenDevices_);

    // Help menu
    auto *menuHelp = menuBar()->addMenu(tr("&Help"));

    auto *actionWebsite = new QAction(tr("&Website"), this);
    connect(actionWebsite, &QAction::triggered, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/Tatsh/devmgmt")));
    });
    menuHelp->addAction(actionWebsite);

    menuHelp->addSeparator();

    auto *actionAbout = new QAction(tr("&About Device Manager Viewer..."), this);
    connect(actionAbout, &QAction::triggered, this, &ViewerMainWindow::about);
    menuHelp->addAction(actionAbout);
}

bool ViewerMainWindow::openFile() {
    QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Device Manager Export"),
        defaultDir,
        tr("Device Manager Export (*%1);;All Files (*)")
            .arg(QLatin1String(DeviceExport::FILE_EXTENSION)));

    if (filePath.isEmpty()) {
        return false;
    }

    return loadFile(filePath);
}

void ViewerMainWindow::openNewFile() {
    openFile();
}

bool ViewerMainWindow::loadFile(const QString &filePath) {
    if (!ExportedData::instance().loadFromFile(filePath)) {
        QMessageBox::warning(this,
                             tr("Error"),
                             tr("Failed to load export file:\n%1").arg(filePath));
        return false;
    }

    updateWindowTitle();

    // Switch to default view
    currentViewAction_ = actionDevicesByType_;
    actionDevicesByType_->setChecked(true);
    switchToDevicesByType();

    return true;
}

void ViewerMainWindow::updateWindowTitle() {
    auto &data = ExportedData::instance();
    if (data.isLoaded()) {
        QFileInfo fi(data.currentFilePath());
        setWindowTitle(tr("Device Manager Viewer - %1 [%2]")
                           .arg(fi.fileName(), data.hostname()));
    } else {
        setWindowTitle(tr("Device Manager Viewer"));
    }
}

void ViewerMainWindow::switchToModel(QAbstractItemModel *model, int depth) {
    auto *oldModel = treeView_->model();
    treeView_->setModel(model);
    if (oldModel && oldModel != model) {
        oldModel->deleteLater();
    }
    if (model != nullptr) {
        treeView_->expandToDepth(depth);
    }
    applyViewSettings();
}

void ViewerMainWindow::switchToDevicesByType() {
    auto *oldModel = treeView_->model();
    auto *model = new ViewerDevicesByTypeModel(this);
    treeView_->setModel(model);
    if (oldModel && oldModel != model) {
        oldModel->deleteLater();
    }
    treeView_->expandToDepth(0);
    applyViewSettings();
}

void ViewerMainWindow::applyViewSettings() {
    // Show driver column only for Devices by Type view
    bool showDriverColumn = (currentViewAction_ == actionDevicesByType_);

    if (showDriverColumn) {
        treeView_->header()->setVisible(true);
        treeView_->setHeaderHidden(false);
        treeView_->header()->setStretchLastSection(false);
        treeView_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        treeView_->header()->setSectionResizeMode(1, QHeaderView::Interactive);
        treeView_->header()->setMinimumSectionSize(100);
        treeView_->setColumnWidth(1, 150);
    } else {
        treeView_->header()->setVisible(false);
        treeView_->setHeaderHidden(true);
        treeView_->header()->setStretchLastSection(false);
        treeView_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        for (int i = 1; i < treeView_->header()->count(); ++i) {
            treeView_->setColumnHidden(i, true);
        }
    }
}

void ViewerMainWindow::openPropertiesForIndex(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto *node = static_cast<Node *>(index.internalPointer());
    if (node && node->type() == NodeType::Device && !node->syspath().isEmpty()) {
        ViewerPropertiesDialog dialog(this);
        dialog.setDeviceSyspath(node->syspath());
        dialog.setCategoryIcon(node->icon());
        dialog.exec();
    }
}

bool ViewerMainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == treeView_ && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QModelIndex index = treeView_->currentIndex();
            if (index.isValid()) {
                auto *node = static_cast<Node *>(index.internalPointer());
                if (node && node->type() == NodeType::Device) {
                    openPropertiesForIndex(index);
                    return true;
                } else {
                    // Toggle expand/collapse for category nodes
                    if (treeView_->isExpanded(index)) {
                        treeView_->collapse(index);
                    } else {
                        treeView_->expand(index);
                    }
                    return true;
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void ViewerMainWindow::about() {
    auto &data = ExportedData::instance();
    QString info;
    if (data.isLoaded()) {
        info = tr("<p>Viewing export from: <b>%1</b></p>"
                  "<p>Export date: %2</p>"
                  "<p>Source: %3 %4</p>")
                   .arg(data.hostname(), data.exportDate(), data.sourceApplicationName(),
                        data.sourceApplicationVersion());
    } else {
        info = tr("<p>No file loaded.</p>");
    }

    QMessageBox::about(this,
                       tr("About Device Manager Viewer"),
                       tr("<h3>Device Manager Viewer</h3>"
                          "<p>View exported device manager data for debugging.</p>"
                          "%1")
                           .arg(info));
}

void ViewerMainWindow::toggleShowHiddenDevices() {
    ExportedData::instance().setShowHiddenDevices(actionShowHiddenDevices_->isChecked());

    // Save expanded state before rebuilding
    QSet<QString> expandedPaths = saveExpandedState();

    // Rebuild the current view
    if (currentViewAction_) {
        currentViewAction_->trigger();
    }

    // Restore expanded state
    restoreExpandedState(expandedPaths);
}

QSet<QString> ViewerMainWindow::saveExpandedState() const {
    QSet<QString> expandedPaths;
    auto *model = treeView_->model();
    if (!model) {
        return expandedPaths;
    }
    collectExpandedPaths(QModelIndex(), QString(), expandedPaths);
    return expandedPaths;
}

void ViewerMainWindow::collectExpandedPaths(const QModelIndex &parent,
                                            const QString &parentPath,
                                            QSet<QString> &expandedPaths) const {
    auto *model = treeView_->model();
    if (!model) {
        return;
    }

    int rowCount = model->rowCount(parent);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0, parent);
        if (!index.isValid()) {
            continue;
        }

        QString name = model->data(index, Qt::DisplayRole).toString();
        QString path = parentPath.isEmpty() ? name : parentPath + QStringLiteral("/") + name;

        if (treeView_->isExpanded(index)) {
            expandedPaths.insert(path);
            collectExpandedPaths(index, path, expandedPaths);
        }
    }
}

void ViewerMainWindow::restoreExpandedState(const QSet<QString> &expandedPaths) {
    if (expandedPaths.isEmpty()) {
        return;
    }
    expandMatchingPaths(QModelIndex(), QString(), expandedPaths);
}

void ViewerMainWindow::expandMatchingPaths(const QModelIndex &parent,
                                           const QString &parentPath,
                                           const QSet<QString> &expandedPaths) {
    auto *model = treeView_->model();
    if (!model) {
        return;
    }

    int rowCount = model->rowCount(parent);
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index = model->index(row, 0, parent);
        if (!index.isValid()) {
            continue;
        }

        QString name = model->data(index, Qt::DisplayRole).toString();
        QString path = parentPath.isEmpty() ? name : parentPath + QStringLiteral("/") + name;

        if (expandedPaths.contains(path)) {
            treeView_->expand(index);
            expandMatchingPaths(index, path, expandedPaths);
        }
    }
}
