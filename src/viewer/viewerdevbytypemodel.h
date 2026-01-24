#pragma once

#include "models/basetreemodel.h"

/**
 * @brief Model for displaying exported devices organized by type/category.
 */
class ViewerDevicesByTypeModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit ViewerDevicesByTypeModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

protected:
    QVariant decorationData(Node *item, int column) const override;

private:
    void buildTree();
    void finalizeCategory(Node *&category);

    Node *hostnameItem;
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
