#ifndef DEVBYTYPEMODEL_H
#define DEVBYTYPEMODEL_H

#include <QtCore/QAbstractItemModel>

#include "node.h"

class DevicesByTypeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit DevicesByTypeModel(QObject *parent = nullptr);
    ~DevicesByTypeModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex
    index(int row,
          int column,
          const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    QString hostname;
    Node *rootItem;
    // Categories
    Node *audioInputsAndOutputsItem;
    Node *batteriesItem;
    Node *computerItem;
    Node *diskDrivesItem;
    Node *displayAdaptersItem;
    Node *dvdCDROMDrivesItem;
    Node *humanInterfaceDevicesItem;
    Node *ideATAATAPIControllersItem;
    Node *keyboardsItem;
    Node *miceAndOtherPointingDevicesItem;
    Node *monitorsItem;
    Node *networkAdaptersItem;
    Node *printQueuesItem;
    Node *processorsItem;
    Node *softwareComponentsItem;
    Node *softwareDevicesItem;
    Node *soundVideoAndGameControllersItem;
    Node *storageControllersItem;
    Node *storageVolumesItem;
    Node *systemDevicesItem;
    Node *universalSerialBusControllersItem;
};

#endif // DEVBYTYPEMODEL_H
