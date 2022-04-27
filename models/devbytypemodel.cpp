#include <QtCore/QRegularExpression>
#include <QtDebug>
#include <libudev.h>

#include "../udev.h"
#include "devbytypemodel.h"

DevicesByTypeModel::DevicesByTypeModel(QObject *parent)
    : QAbstractItemModel(parent) {
    udevManager manager;

    rootItem = new Node({QStringLiteral("")});
    auto hostnameItem = new Node({QStringLiteral("HOSTNAME")}, rootItem);
    hostnameItem->setIcon(QIcon::fromTheme(QStringLiteral("computer")));
    rootItem->appendChild(hostnameItem);
    hostnameItem->appendChild(
        audioInputsAndOutputsItem =
            new Node({tr("Audio inputs and outputs")}, hostnameItem));
    audioInputsAndOutputsItem->setIcon(
        QIcon::fromTheme(QStringLiteral("audio-card")));

    audioInputsAndOutputsItem->appendChild(new Node(
        {tr("Fake sound card")}, audioInputsAndOutputsItem, NodeType::Device));

    hostnameItem->appendChild(batteriesItem =
                                  new Node({tr("Batteries")}, hostnameItem));
    batteriesItem->setIcon(QIcon::fromTheme(QStringLiteral("battery-ups")));
    hostnameItem->appendChild(computerItem =
                                  new Node({tr("Computer")}, hostnameItem));
    computerItem->setIcon(QIcon::fromTheme(QStringLiteral("computer")));

    hostnameItem->appendChild(diskDrivesItem =
                                  new Node({tr("Disk drives")}, hostnameItem));
    diskDrivesItem->setIcon(
        QIcon::fromTheme(QStringLiteral("drive-harddisk")));
    for (DeviceInfo info : manager.iterDevicesSubsystem("block")) {
        if (info.propertyValue("DEVTYPE") == QStringLiteral("partition") ||
            info.propertyValue("ID_CDROM") == QStringLiteral("1") ||
            info.propertyValue("DEVPATH").startsWith(
                QStringLiteral("/devices/virtual/"))) {
            continue;
        }
        diskDrivesItem->appendChild(new Node(
            {info.name(), info.driver()}, diskDrivesItem, NodeType::Device));
    }

    hostnameItem->appendChild(displayAdaptersItem = new Node(
                                  {tr("Display adapters")}, hostnameItem));
    displayAdaptersItem->setIcon(
        QIcon::fromTheme(QStringLiteral("video-display")));
    auto enumerator = udev_enumerate_new(manager.context());
    Q_ASSERT(enumerator);
    udev_enumerate_add_match_property(
        enumerator, "ID_PCI_CLASS_FROM_DATABASE", "Display controller");
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        displayAdaptersItem->appendChild(new Node({info.name(), info.driver()},
                                                  displayAdaptersItem,
                                                  NodeType::Device));
    }
    udev_enumerate_unref(enumerator);

    hostnameItem->appendChild(dvdCDROMDrivesItem = new Node(
                                  {tr("DVD/CD-ROM drives")}, hostnameItem));
    dvdCDROMDrivesItem->setIcon(
        QIcon::fromTheme(QStringLiteral("drive-optical")));
    enumerator = udev_enumerate_new(manager.context());
    Q_ASSERT(enumerator);
    udev_enumerate_add_match_property(enumerator, "ID_CDROM", "1");
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            continue;
        }
        dvdCDROMDrivesItem->appendChild(new Node({info.name(), info.driver()},
                                                 dvdCDROMDrivesItem,
                                                 NodeType::Device));
    }
    udev_enumerate_unref(enumerator);

    hostnameItem->appendChild(
        humanInterfaceDevicesItem =
            new Node({tr("Human Interface Devices")}, hostnameItem));
    humanInterfaceDevicesItem->setIcon(
        QIcon::fromTheme(QStringLiteral("input-tablet")));
    for (DeviceInfo info : manager.iterDevicesSubsystem("hid")) {
        humanInterfaceDevicesItem->appendChild(
            new Node({info.name(), info.driver()},
                     humanInterfaceDevicesItem,
                     NodeType::Device));
    }

    hostnameItem->appendChild(
        ideATAATAPIControllersItem =
            new Node({tr("IDE ATA/ATAPI controllers")}, hostnameItem));
    ideATAATAPIControllersItem->setIcon(
        QIcon::fromTheme(QStringLiteral("drive-harddisk")));

    hostnameItem->appendChild(keyboardsItem =
                                  new Node({tr("Keyboards")}, hostnameItem));
    keyboardsItem->setIcon(QIcon::fromTheme(QStringLiteral("input-keyboard")));
    enumerator = udev_enumerate_new(manager.context());
    Q_ASSERT(enumerator);
    udev_enumerate_add_match_property(enumerator, "ID_INPUT_KEYBOARD", "1");
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            continue;
        }
        keyboardsItem->appendChild(new Node(
            {info.name(), info.driver()}, keyboardsItem, NodeType::Device));
    }
    udev_enumerate_unref(enumerator);

    hostnameItem->appendChild(
        miceAndOtherPointingDevicesItem =
            new Node({tr("Mice and other pointing devices")}, hostnameItem));
    miceAndOtherPointingDevicesItem->setIcon(
        QIcon::fromTheme(QStringLiteral("input-mouse")));
    enumerator = udev_enumerate_new(manager.context());
    Q_ASSERT(enumerator);
    udev_enumerate_add_match_property(enumerator, "ID_INPUT_MOUSE", "1");
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            continue;
        }
        miceAndOtherPointingDevicesItem->appendChild(
            new Node({info.name(), info.driver()},
                     miceAndOtherPointingDevicesItem,
                     NodeType::Device));
    }
    udev_enumerate_unref(enumerator);

    hostnameItem->appendChild(monitorsItem =
                                  new Node({tr("Monitors")}, hostnameItem));
    monitorsItem->setIcon(QIcon::fromTheme(QStringLiteral("monitor")));

    hostnameItem->appendChild(networkAdaptersItem = new Node(
                                  {tr("Network adapters")}, hostnameItem));
    networkAdaptersItem->setIcon(
        QIcon::fromTheme(QStringLiteral("network-wired")));

    hostnameItem->appendChild(
        printQueuesItem = new Node({tr("Print queues")}, hostnameItem));
    printQueuesItem->setIcon(QIcon::fromTheme(QStringLiteral("printer")));

    hostnameItem->appendChild(processorsItem =
                                  new Node({tr("Processors")}, hostnameItem));
    processorsItem->setIcon(
        QIcon::fromTheme(QStringLiteral("preferences-devices-cpu")));

    hostnameItem->appendChild(softwareComponentsItem = new Node(
                                  {tr("Software components")}, hostnameItem));
    softwareComponentsItem->setIcon(
        QIcon::fromTheme(QStringLiteral("preferences-other")));

    hostnameItem->appendChild(softwareDevicesItem = new Node(
                                  {tr("Software devices")}, hostnameItem));
    softwareDevicesItem->setIcon(
        QIcon::fromTheme(QStringLiteral("preferences-other")));
    for (DeviceInfo info : manager.iterDevicesSubsystem("misc")) {
        softwareDevicesItem->appendChild(new Node(
            {info.name().replace(QRegularExpression(QStringLiteral("^/dev/")),
                                 QStringLiteral("")),
             info.driver()},
            softwareDevicesItem,
            NodeType::Device));
    }

    hostnameItem->appendChild(
        soundVideoAndGameControllersItem =
            new Node({tr("Sound, video and game controllers")}, hostnameItem));
    soundVideoAndGameControllersItem->setIcon(
        QIcon::fromTheme(QStringLiteral("preferences-desktop-sound")));

    hostnameItem->appendChild(storageControllersItem = new Node(
                                  {tr("Storage controllers")}, hostnameItem));
    storageControllersItem->setIcon(
        QIcon::fromTheme(QStringLiteral("drive-harddisk")));
    enumerator = udev_enumerate_new(manager.context());
    Q_ASSERT(enumerator);
    udev_enumerate_add_match_property(
        enumerator, "ID_PCI_CLASS_FROM_DATABASE", "Mass storage controller");
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            continue;
        }
        storageControllersItem->appendChild(
            new Node({info.name(), info.driver()},
                     storageControllersItem,
                     NodeType::Device));
    }
    udev_enumerate_unref(enumerator);

    hostnameItem->appendChild(
        storageVolumesItem = new Node({tr("Storage volumes")}, hostnameItem));
    storageVolumesItem->setIcon(
        QIcon::fromTheme(QStringLiteral("drive-partition")));
    enumerator = udev_enumerate_new(manager.context());
    Q_ASSERT(enumerator);
    udev_enumerate_add_match_property(enumerator, "DEVTYPE", "partition");
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        storageVolumesItem->appendChild(new Node({info.name(), info.driver()},
                                                 storageVolumesItem,
                                                 NodeType::Device));
    }
    udev_enumerate_unref(enumerator);

    hostnameItem->appendChild(
        systemDevicesItem = new Node({tr("System devices")}, hostnameItem));
    systemDevicesItem->setIcon(QIcon::fromTheme(QStringLiteral("computer")));
    for (DeviceInfo info : manager.iterDevicesSubsystem("pci")) {
        if (info.name().isEmpty() ||
            info.propertyValue("ID_PCI_CLASS_FROM_DATABASE") ==
                QStringLiteral("Display controller") ||
            info.propertyValue("ID_PCI_SUBCLASS_FROM_DATABASE") ==
                QStringLiteral("Audio device") ||
            info.propertyValue("ID_PCI_INTERFACE_FROM_DATABASE") ==
                QStringLiteral("NVM Express") ||
            info.propertyValue("ID_PCI_SUBCLASS_FROM_DATABASE") ==
                QStringLiteral("USB controller") ||
            info.propertyValue("ID_PCI_SUBCLASS_FROM_DATABASE") ==
                QStringLiteral("SATA controller") ||
            info.propertyValue("ID_PCI_CLASS_FROM_DATABASE") ==
                QStringLiteral("Network controller")) {
            continue;
        }
        systemDevicesItem->appendChild(new Node({info.name(), info.driver()},
                                                systemDevicesItem,
                                                NodeType::Device));
    }
    // for (DeviceInfo info : manager.iterDevicesSubsystem("mem")) {
    //     if (info.name().isEmpty()) {
    //         continue;
    //     }
    //     systemDevicesItem->appendChild(new Node({info.name(),
    //     info.driver()},
    //                                             systemDevicesItem,
    //                                             NodeType::Device));
    // }

    hostnameItem->appendChild(
        universalSerialBusControllersItem =
            new Node({tr("Universal Serial Bus controllers")}, hostnameItem));
    universalSerialBusControllersItem->setIcon(
        QIcon::fromTheme(QStringLiteral("drive-removable-media-usb")));
    enumerator = udev_enumerate_new(manager.context());
    Q_ASSERT(enumerator);
    udev_enumerate_add_match_property(
        enumerator, "ID_PCI_SUBCLASS_FROM_DATABASE", "USB controller");
    for (DeviceInfo info : manager.scanDevices(enumerator)) {
        if (info.name().isEmpty()) {
            continue;
        }
        universalSerialBusControllersItem->appendChild(
            new Node({info.name(), info.driver()},
                     universalSerialBusControllersItem,
                     NodeType::Device));
    }
    udev_enumerate_unref(enumerator);
}

DevicesByTypeModel::~DevicesByTypeModel() {
    delete rootItem;
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
    Node *childItem = parentItem->child(row);
    if (childItem) {
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
