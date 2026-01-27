// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include "basetreemodel.h"

/**
 * @brief Tree model that organises devices by their driver.
 *
 * This model displays devices grouped under their kernel driver names.
 * Devices using the same driver are shown together under a driver node.
 */
class DevicesByDriverModel : public BaseTreeModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DevicesByDriverModel.
     * @param parent Optional parent QObject.
     */
    explicit DevicesByDriverModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void buildTree();

    Node *hostnameItem;
};
