// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include "basetreemodel.h"

/**
 * @brief Tree model that organises devices by their category/type.
 *
 * This model displays devices grouped under category nodes such as
 * "Disk Drives", "Network Adapters", "USB Controllers", etc.
 * It provides a two-column view with device name and driver.
 */
class DevicesByTypeModel : public BaseTreeModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DevicesByTypeModel.
     * @param parent Optional parent QObject.
     */
    explicit DevicesByTypeModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

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
