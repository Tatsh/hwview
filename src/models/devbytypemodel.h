#pragma once

#include "basetreemodel.h"

class DevicesByTypeModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit DevicesByTypeModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    QVariant decorationData(Node *item, int column) const override;

private:
    void buildTree();
    void finalizeCategory(Node *&category);

    Node *hostnameItem;
    // Categories
    Node *audioInputsAndOutputsItem;
    Node *batteriesItem;
    Node *computerItem;
    Node *diskDrivesItem;
    Node *displayAdaptersItem;
    Node *dvdCdromDrivesItem;
    Node *humanInterfaceDevicesItem;
    Node *keyboardsItem;
    Node *miceAndOtherPointingDevicesItem;
    Node *networkAdaptersItem;
    Node *softwareDevicesItem;
    Node *soundVideoAndGameControllersItem;
    Node *storageControllersItem;
    Node *storageVolumesItem;
    Node *systemDevicesItem;
    Node *universalSerialBusControllersItem;
};
