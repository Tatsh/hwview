// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include "basetreemodel.h"

/**
 * @brief Tree model that organises drivers by the devices they support.
 *
 * This model displays drivers as parent nodes with their supported
 * devices as children. This shows the one-to-many relationship between
 * drivers and devices.
 */
class DriversByDeviceModel : public BaseTreeModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DriversByDeviceModel.
     * @param parent Optional parent QObject.
     */
    explicit DriversByDeviceModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void buildTree();

    Node *hostnameItem;
};
