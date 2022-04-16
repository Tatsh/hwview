#include <QtWidgets/QAction>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeView>

#include "mainwindow.h"
#include "models/devbytypemodel.h"

MainWindow::MainWindow() : treeView(new QTreeView) {
    setupTreeView();
    createActions();
    statusBar()->showMessage(tr("Ready"));
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::setupTreeView() {
    setCentralWidget(treeView);
    treeView->setHeaderHidden(true);
    treeView->setModel(new DevicesByTypeModel(this));
    treeView->expandToDepth(1);
    treeView->show();
}

void MainWindow::about() {
    QMessageBox::about(this,
                       tr("About Device Manager"),
                       tr("View and manage device hardware settings and "
                          "driver software installed on your computer."));
}

void MainWindow::createActions() {
    auto fileMenu = menuBar()->addMenu(tr("&File"));
    auto optionsAction = fileMenu->addAction(tr("Options..."));
    optionsAction->setShortcuts(QKeySequence::Preferences);
    auto exitAction = fileMenu->addAction(tr("Exit"));
    exitAction->setShortcuts(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    auto actionMenu = menuBar()->addMenu(tr("&Action"));
    updateDriverAction = actionMenu->addAction(tr("Update driver"));
    updateDriverAction->setVisible(false);
    updateDriverSeparatorAction = actionMenu->addSeparator();
    updateDriverSeparatorAction->setVisible(false);
    actionMenu->addAction(tr("Add drivers"));
    actionMenu->addSeparator();
    actionMenu->addAction(tr("Scan for hardware changes"));
    actionMenu->addAction(tr("Add legacy hardware"));
    actionMenu->addSeparator();
    actionMenu->addAction(tr("Devices and Printers"));
    actionMenu->addSeparator();
    actionMenu->addAction(tr("Help"));

    auto viewMenu = menuBar()->addMenu(tr("&View"));
    auto viewTypeActionGroup = new QActionGroup(this);
    // Devices by type
    auto devicesByTypeAction =
        viewTypeActionGroup->addAction(tr("D&evices by type"));
    devicesByTypeAction->setCheckable(true);
    devicesByTypeAction->setChecked(true);
    viewMenu->addAction(devicesByTypeAction);
    // Devices by connection
    auto devicesByConnectionAction =
        viewTypeActionGroup->addAction(tr("De&vices by connection"));
    devicesByConnectionAction->setCheckable(true);
    viewMenu->addAction(devicesByConnectionAction);
    // Devices by container
    auto devicesByContainerAction =
        viewTypeActionGroup->addAction(tr("Devi&ces by container"));
    devicesByContainerAction->setCheckable(true);
    viewMenu->addAction(devicesByContainerAction);
    // Devices by driver
    auto devicesByDriverAction =
        viewTypeActionGroup->addAction(tr("Dev&ices by driver"));
    devicesByDriverAction->setCheckable(true);
    viewMenu->addAction(devicesByDriverAction);
    // Drivers by type
    auto driversByTypeAction =
        viewTypeActionGroup->addAction(tr("Drive&rs by type"));
    driversByTypeAction->setCheckable(true);
    viewMenu->addAction(driversByTypeAction);
    // Drivers by device
    auto driversByDeviceAction =
        viewTypeActionGroup->addAction(tr("Driver&s by device"));
    driversByDeviceAction->setCheckable(true);
    viewMenu->addAction(driversByDeviceAction);
    // Resources by type
    auto resourcesByTypeAction =
        viewTypeActionGroup->addAction(tr("Resources by t&ype"));
    resourcesByTypeAction->setCheckable(true);
    viewMenu->addAction(resourcesByTypeAction);
    // Resources by connection
    auto resourcesByConnectionAction =
        viewTypeActionGroup->addAction(tr("Resources by co&nnection"));
    resourcesByConnectionAction->setCheckable(true);
    viewMenu->addAction(resourcesByConnectionAction);
    viewMenu->addSeparator();
    // Removes empty categories, hides disconnected devices
    auto showHiddenDevicesAction =
        viewMenu->addAction(tr("Sho&w hidden devices"));
    showHiddenDevicesAction->setCheckable(true);
    viewMenu->addSeparator();
    viewMenu->addAction(tr("C&ustomize..."));

    auto helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("Help Topics"));
    helpMenu->addAction(tr("Website"));
    helpMenu->addSeparator();
    auto aboutAct = helpMenu->addAction(
        tr("&About Device Manager..."), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));
}

/*
Toolbar:
Back forward | (Show/Hide Console Tree) | Properties | Help (Show/Hide Action
Pane) | Add drivers | Scan for hardware changes

On selection of category: | Properties | after (Show/Hide Console Tree)
On selection of device, at end: (Update device driver) | (Uninstall device) |
(Disable device)
*/

/*
  Devices by type
  + hostname
    -> Audio inputs and outputs
    -> Batteries
    -> Computer
    -> Disk Drives
    -> DVD/CD-ROM drives
    -> Human Interface Devices
    -> IDE ATA/ATAPI controllers
    -> Keyboards
    -> Mice and other pointing devices
    -> Monitors
    -> Network adapters
    -> Print queues
    -> Processors
    -> Software components
    -> Software devices
    -> Sound, video and game controllers
    -> Storage controllers
    -> Storage volumes
    -> System devices
    -> Universal Serial Bus controllers

Drivers by type is same as above but with driver names before the device list
(more often just one device per driver) Drivers by device is same as Drivers by
type but in reverse (usually just one driver per device)

Resources by type (ranges given in uppercase hex but without 0x prefix):
+ Hostname
  -> Direct memory access (DMA)
     -> ? Direct memory access controller
  -> Input/output (IO)
     [range - range] Name of device
  -> Interrupt request (IRQ)
     (type: e.g PCI or ISA) 0x ADDRESS (?) Name of device
  -> Large memory
     [range - range] name of device
  -> Memory
     [range - range] name of device
Resources by connection is similar to above except with more hierarchy in
Input/output (IO).
*/
