#include <QtWidgets/QAction>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeView>

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "models/devbytypemodel.h"

MainWindow::MainWindow() {
    setupUi(this);
    setUnifiedTitleAndToolBarOnMac(true);
    connect(actionAbout, &QAction::triggered, this, &MainWindow::about);

    actionGroupView = new QActionGroup(this);
    actionGroupView->addAction(actionDevicesByType);
    actionGroupView->addAction(actionDevicesByConnection);
    actionGroupView->addAction(actionDevicesByContainer);
    actionGroupView->addAction(actionDevicesByDriver);
    actionGroupView->addAction(actionDriversByType);
    actionGroupView->addAction(actionDriversByDevice);
    actionGroupView->addAction(actionResourcesByType);
    actionGroupView->addAction(actionResourcesByConnection);
    actionGroupView->setExclusive(true);

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
