#include "devbytypemodel.h"

#include <QtDebug>

DevicesByTypeModel::DevicesByTypeModel(QObject *parent)
    : QAbstractItemModel(parent) {
    rootItem = new Node({QStringLiteral("")});
    auto hostnameItem = new Node({QStringLiteral("HOSTNAME")}, rootItem);
    hostnameItem->setIcon(QIcon::fromTheme(QStringLiteral("computer")));
    rootItem->appendChild(hostnameItem);
    hostnameItem->appendChild(
        audioInputsAndOutputsItem =
            new Node({tr("Audio inputs and outputs")}, hostnameItem));
    audioInputsAndOutputsItem->setIcon(
        QIcon::fromTheme(QStringLiteral("audio-card")));
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
    hostnameItem->appendChild(displayAdaptersItem = new Node(
                                  {tr("Display adapters")}, hostnameItem));
    displayAdaptersItem->setIcon(
        QIcon::fromTheme(QStringLiteral("video-display")));
    hostnameItem->appendChild(dvdCDROMDrivesItem = new Node(
                                  {tr("DVD/CD-ROM drives")}, hostnameItem));
    dvdCDROMDrivesItem->setIcon(
        QIcon::fromTheme(QStringLiteral("drive-optical")));
    hostnameItem->appendChild(
        ideATAATAPIControllersItem =
            new Node({tr("IDE ATA/ATAPI controllers")}, hostnameItem));
    hostnameItem->appendChild(keyboardsItem =
                                  new Node({tr("Keyboards")}, hostnameItem));
    hostnameItem->appendChild(
        miceAndOtherPointingDevicesItem =
            new Node({tr("Mice and other pointing devices")}, hostnameItem));
    hostnameItem->appendChild(monitorsItem =
                                  new Node({tr("Monitors")}, hostnameItem));
    hostnameItem->appendChild(networkAdaptersItem = new Node(
                                  {tr("Network adapters")}, hostnameItem));
    hostnameItem->appendChild(
        printQueuesItem = new Node({tr("Print queues")}, hostnameItem));
    hostnameItem->appendChild(processorsItem =
                                  new Node({tr("Processors")}, hostnameItem));
    hostnameItem->appendChild(softwareComponentsItem = new Node(
                                  {tr("Software components")}, hostnameItem));
    hostnameItem->appendChild(softwareDevicesItem = new Node(
                                  {tr("Software devices")}, hostnameItem));
    hostnameItem->appendChild(
        soundVideoAndGameControllersItem =
            new Node({tr("Sound, video and game controllers")}, hostnameItem));
    hostnameItem->appendChild(storageControllersItem = new Node(
                                  {tr("Storage controllers")}, hostnameItem));
    hostnameItem->appendChild(
        storageVolumesItem = new Node({tr("Storage volumes")}, hostnameItem));
    hostnameItem->appendChild(
        systemDevicesItem = new Node({tr("System devices")}, hostnameItem));
    hostnameItem->appendChild(
        storageVolumesItem =
            new Node({tr("Universal Serial Bus controllers")}, hostnameItem));
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
