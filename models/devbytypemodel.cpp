#include <QtCore/QRegularExpression>
#include <QtDebug>
#include <QtNetwork/QHostInfo>
#include <libudev.h>

#include "const_strings.h"
#include "models/devbytypemodel.h"

namespace s = strings;
namespace us = strings::udev;

DevicesByTypeModel::DevicesByTypeModel(QObject *parent)
    : QAbstractItemModel(parent), manager(UdevManager()),
      rootItem(new Node({strings::empty})),
      hostnameItem(new Node({QHostInfo::localHostName()}, rootItem)) {
    hostnameItem->setIconFromTheme(s::categoryIcons::computer);
    rootItem->appendChild(hostnameItem);
    addAudio();
    addBatteries();
    addComputer();
    addDiskDrives();
    addDisplayAdapters();
    addOptical();
    addHid();
    addIdeAtapi();
    addKeyboards();
    addMice();
    addMonitors();
    addNetwork();
    addPrintQueues();
    addProcessors();
    addSoftwareComponents();
    addSoftwareDevices();
    addSoundVideoAndGameControllers();
    addStorageControllers();
    addStorageVolumes();
    addSystemDevices();
    addUsbControllers();
}

DevicesByTypeModel::~DevicesByTypeModel() {
    delete rootItem;
}

void DevicesByTypeModel::addAudio() {
    hostnameItem->appendChild(
        audioInputsAndOutputsItem =
            new Node({tr("Audio inputs and outputs")}, hostnameItem));
    audioInputsAndOutputsItem->setIconFromTheme(s::categoryIcons::audioInputs);
}

void DevicesByTypeModel::addBatteries() {
    hostnameItem->appendChild(batteriesItem =
                                  new Node({tr("Batteries")}, hostnameItem));
    batteriesItem->setIconFromTheme(s::categoryIcons::batteries);
}

void DevicesByTypeModel::addComputer() {
    // /devices/virtual/dmi/id, exclude form system devices
    hostnameItem->appendChild(computerItem =
                                  new Node({tr("Computer")}, hostnameItem));
    computerItem->setIconFromTheme(s::categoryIcons::computer);
}

void DevicesByTypeModel::addDiskDrives() {
    hostnameItem->appendChild(diskDrivesItem =
                                  new Node({tr("Disk drives")}, hostnameItem));
    diskDrivesItem->setIconFromTheme(s::categoryIcons::diskDrives);
    for (DeviceInfo info :
         manager.iterDevicesSubsystem(us::subsystems::block)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        if (info.propertyValue(us::propertyNames::DEVTYPE) ==
                us::propertyValues::devType::partition ||
            info.propertyValue(us::propertyNames::ID_CDROM) == s::digit1 ||
            info.propertyValue(us::propertyNames::DEVPATH)
                .startsWith(QStringLiteral("/devices/virtual/"))) {
            continue;
        }
        diskDrivesItem->appendChild(new Node(
            {info.name(), info.driver()}, diskDrivesItem, NodeType::Device));
    }
}

void DevicesByTypeModel::addDisplayAdapters() {
    hostnameItem->appendChild(displayAdaptersItem = new Node(
                                  {tr("Display adapters")}, hostnameItem));
    displayAdaptersItem->setIconFromTheme(s::categoryIcons::displayAdapters);
    auto enumerator = std::make_unique<UdevEnumerate>(manager);
    enumerator->addMatchProperty(
        us::propertyNames::ID_PCI_CLASS_FROM_DATABASE,
        us::propertyValues::idPciClassFromDatabase::displayController);
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        displayAdaptersItem->appendChild(new Node({info.name(), info.driver()},
                                                  displayAdaptersItem,
                                                  NodeType::Device));
    }
}

void DevicesByTypeModel::addOptical() {
    hostnameItem->appendChild(dvdCdromDrivesItem = new Node(
                                  {tr("DVD/CD-ROM drives")}, hostnameItem));
    dvdCdromDrivesItem->setIconFromTheme(s::categoryIcons::dvdCdromDrives);
    auto enumerator = std::make_unique<UdevEnumerate>(manager);
    enumerator->addMatchProperty(us::propertyNames::ID_CDROM);
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        dvdCdromDrivesItem->appendChild(new Node({info.name(), info.driver()},
                                                 dvdCdromDrivesItem,
                                                 NodeType::Device));
    }
}

void DevicesByTypeModel::addHid() {
    hostnameItem->appendChild(
        humanInterfaceDevicesItem =
            new Node({tr("Human Interface Devices")}, hostnameItem));
    humanInterfaceDevicesItem->setIconFromTheme(s::categoryIcons::hid);
    for (DeviceInfo info : manager.iterDevicesSubsystem(us::subsystems::hid)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        humanInterfaceDevicesItem->appendChild(
            new Node({info.name(), info.driver()},
                     humanInterfaceDevicesItem,
                     NodeType::Device));
    }
}

void DevicesByTypeModel::addIdeAtapi() {
    hostnameItem->appendChild(
        ideAtaAtapiControllersItem =
            new Node({tr("IDE ATA/ATAPI controllers")}, hostnameItem));
    ideAtaAtapiControllersItem->setIconFromTheme(
        s::categoryIcons::ideAtapiControllers);
}

void DevicesByTypeModel::addKeyboards() {
    hostnameItem->appendChild(keyboardsItem =
                                  new Node({tr("Keyboards")}, hostnameItem));
    keyboardsItem->setIconFromTheme(s::categoryIcons::keyboards);
    auto enumerator = std::make_unique<UdevEnumerate>(manager);
    enumerator->addMatchProperty(us::propertyNames::ID_INPUT_KEYBOARD);
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        keyboardsItem->appendChild(new Node(
            {info.name(), info.driver()}, keyboardsItem, NodeType::Device));
    }
}

void DevicesByTypeModel::addMice() {
    hostnameItem->appendChild(
        miceAndOtherPointingDevicesItem =
            new Node({tr("Mice and other pointing devices")}, hostnameItem));
    miceAndOtherPointingDevicesItem->setIconFromTheme(s::categoryIcons::mice);
    auto enumerator = std::make_unique<UdevEnumerate>(manager);
    enumerator->addMatchProperty(us::propertyNames::ID_INPUT_MOUSE);
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        miceAndOtherPointingDevicesItem->appendChild(
            new Node({info.name(), info.driver()},
                     miceAndOtherPointingDevicesItem,
                     NodeType::Device));
    }
}

void DevicesByTypeModel::addMonitors() {
    hostnameItem->appendChild(monitorsItem =
                                  new Node({tr("Monitors")}, hostnameItem));
    monitorsItem->setIconFromTheme(s::categoryIcons::monitor);
}

void DevicesByTypeModel::addNetwork() {
    hostnameItem->appendChild(networkAdaptersItem = new Node(
                                  {tr("Network adapters")}, hostnameItem));
    networkAdaptersItem->setIconFromTheme(s::categoryIcons::networkAdapters);
}

void DevicesByTypeModel::addPrintQueues() {
    hostnameItem->appendChild(
        printQueuesItem = new Node({tr("Print queues")}, hostnameItem));
    printQueuesItem->setIconFromTheme(s::categoryIcons::printer);
}

void DevicesByTypeModel::addProcessors() {
    hostnameItem->appendChild(processorsItem =
                                  new Node({tr("Processors")}, hostnameItem));
    processorsItem->setIconFromTheme(s::categoryIcons::processors);
}

void DevicesByTypeModel::addSoftwareComponents() {
    hostnameItem->appendChild(softwareComponentsItem = new Node(
                                  {tr("Software components")}, hostnameItem));
    softwareComponentsItem->setIconFromTheme(s::categoryIcons::other);
}

const QRegularExpression kBeginningWithSlashDevRe(QStringLiteral("^/dev/"));

void DevicesByTypeModel::addSoftwareDevices() {
    hostnameItem->appendChild(softwareDevicesItem = new Node(
                                  {tr("Software devices")}, hostnameItem));
    softwareDevicesItem->setIconFromTheme(s::categoryIcons::other);
    for (DeviceInfo info :
         manager.iterDevicesSubsystem(us::subsystems::misc)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        softwareDevicesItem->appendChild(new Node(
            {info.name().replace(kBeginningWithSlashDevRe, strings::empty),
             info.driver()},
            softwareDevicesItem,
            NodeType::Device));
    }
}

void DevicesByTypeModel::addSoundVideoAndGameControllers() {
    hostnameItem->appendChild(
        soundVideoAndGameControllersItem =
            new Node({tr("Sound, video and game controllers")}, hostnameItem));
    soundVideoAndGameControllersItem->setIconFromTheme(
        s::categoryIcons::soundVideoGameControllers);
}

void DevicesByTypeModel::addStorageControllers() {
    hostnameItem->appendChild(storageControllersItem = new Node(
                                  {tr("Storage controllers")}, hostnameItem));
    storageControllersItem->setIconFromTheme(
        s::categoryIcons::storageControllers);
    auto enumerator = std::make_unique<UdevEnumerate>(manager);
    enumerator->addMatchProperty(
        us::propertyNames::ID_PCI_CLASS_FROM_DATABASE,
        us::propertyValues::idPciClassFromDatabase::massStorageController);
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        storageControllersItem->appendChild(
            new Node({info.name(), info.driver()},
                     storageControllersItem,
                     NodeType::Device));
    }
}

void DevicesByTypeModel::addStorageVolumes() {
    hostnameItem->appendChild(
        storageVolumesItem = new Node({tr("Storage volumes")}, hostnameItem));
    storageVolumesItem->setIconFromTheme(s::categoryIcons::storageVolumes);
    auto enumerator = std::make_unique<UdevEnumerate>(manager);
    enumerator->addMatchProperty(us::propertyNames::DEVTYPE,
                                 us::propertyValues::devType::partition);
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        storageVolumesItem->appendChild(new Node({info.name(), info.driver()},
                                                 storageVolumesItem,
                                                 NodeType::Device));
    }
}

void DevicesByTypeModel::addSystemDevices() {
    hostnameItem->appendChild(
        systemDevicesItem = new Node({tr("System devices")}, hostnameItem));
    systemDevicesItem->setIconFromTheme(s::categoryIcons::systemDevices);
    for (DeviceInfo info : manager.iterDevicesSubsystem(us::subsystems::pci)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        if (info.propertyValue(
                us::propertyNames::ID_PCI_CLASS_FROM_DATABASE) ==
                us::propertyValues::idPciClassFromDatabase::
                    displayController ||
            info.propertyValue(
                us::propertyNames::ID_PCI_SUBCLASS_FROM_DATABASE) ==
                us::propertyValues::idPciSubclassFromDatabase::audioDevice ||
            info.propertyValue(
                us::propertyNames::ID_PCI_INTERFACE_FROM_DATABASE) ==
                us::propertyValues::idPciInterfaceFromDatabase::nvmExpress ||
            info.propertyValue(
                us::propertyNames::ID_PCI_SUBCLASS_FROM_DATABASE) ==
                us::propertyValues::idPciSubclassFromDatabase::usbController ||
            info.propertyValue(
                us::propertyNames::ID_PCI_SUBCLASS_FROM_DATABASE) ==
                us::propertyValues::idPciSubclassFromDatabase::
                    sataController ||
            info.propertyValue(
                us::propertyNames::ID_PCI_CLASS_FROM_DATABASE) ==
                us::propertyValues::idPciClassFromDatabase::
                    networkController ||
            info.propertyValue(
                us::propertyNames::ID_PCI_CLASS_FROM_DATABASE) ==
                us::propertyValues::idPciClassFromDatabase::
                    multimediaController) {
            continue;
        }
        systemDevicesItem->appendChild(new Node({info.name(), info.driver()},
                                                systemDevicesItem,
                                                NodeType::Device));
    }
    // for (DeviceInfo info : manager.iterDevicesSubsystem("mem")) {
    //     systemDevicesItem->appendChild(new Node({info.name(),
    //     info.driver()},
    //                                             systemDevicesItem,
    //                                             NodeType::Device));
    // }
}

void DevicesByTypeModel::addUsbControllers() {
    hostnameItem->appendChild(
        universalSerialBusControllersItem =
            new Node({tr("Universal Serial Bus controllers")}, hostnameItem));
    universalSerialBusControllersItem->setIconFromTheme(
        s::categoryIcons::usbControllers);
    auto enumerator = std::make_unique<UdevEnumerate>(manager);
    enumerator->addMatchProperty(
        strings::udev::propertyNames::ID_PCI_SUBCLASS_FROM_DATABASE,
        us::propertyValues::idPciSubclassFromDatabase::usbController);
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            qDebug() << "Empty name";
            info.dump();
            qDebug() << "END\n";
        }
        universalSerialBusControllersItem->appendChild(
            new Node({info.name(), info.driver()},
                     universalSerialBusControllersItem,
                     NodeType::Device));
    }
}

QModelIndex DevicesByTypeModel::index(int row,
                                      int column,
                                      const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    Node *parentItem;
    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<Node *>(parent.internalPointer());
    }
    Node *childItem;
    if ((childItem = parentItem->child(row))) {
        return createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex DevicesByTypeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }
    Node *childItem = static_cast<Node *>(index.internalPointer());
    Node *parentItem = childItem->parentItem();
    if (parentItem == rootItem) {
        return QModelIndex();
    }
    return createIndex(parentItem->row(), 0, parentItem);
}

int DevicesByTypeModel::rowCount(const QModelIndex &parent) const {
    Node *parentItem;
    if (parent.column() > 0) {
        return 0;
    }
    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<Node *>(parent.internalPointer());
    }
    return parentItem->childCount();
}

int DevicesByTypeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return static_cast<Node *>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

QVariant DevicesByTypeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    Node *item = static_cast<Node *>(index.internalPointer());
    switch (role) {
    case Qt::DecorationRole:
        return item->icon();

    case Qt::DisplayRole:
        return item->data(index.column());

    default:
        return QVariant();
    }
}

Qt::ItemFlags DevicesByTypeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}
