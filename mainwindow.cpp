#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>

#include "mainwindow.h"

MainWindow::MainWindow() {
    createActions();
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::about() {
    QMessageBox::about(this,
                       tr("About Device Manager"),
                       tr("View and manage device hardware settings and "
                          "driver software installed on your computer."));
}

void MainWindow::createActions() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    // Options...
    // Exit

    QMenu *actionMenu = menuBar()->addMenu(tr("&Action"));
    // Add drivers
    // --
    // Scan for hardware changes
    // Add legacy hardware
    // --
    // Devices and Printers
    // --
    // Help

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    // Devices by type
    // Devices by connection
    // Devices by container
    // Devices by driver
    // Drivers by type
    // Drivers by device
    // Resources by type
    // Resources by connection
    // --
    // Show hidden devices (removes empty categories, hides disconnected
    // devices)
    // --
    // Customize...

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    // Help Topics
    // TechCenter Web Site
    // --
    // About Microsoft Management Console...
    QAction *aboutAct = helpMenu->addAction(
        tr("&About Device Manager..."), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));
}

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
    -> Univeral Serial Bus controllers

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
     (type: e.g PCI or ISA) 0xADRESS (?) Name of device
  -> Large memory
     [range - range] name of device
  -> Memory
     [range - range] name of device
Resources by connection is similar to above except with more hierarchy in
Input/output (IO).

Toolbar:
Back forward | (Show/Hide Console Tree) | Properties | Help (Show/Hide Action
Pane) | Add drivers | Scan for hardware changes
*/
