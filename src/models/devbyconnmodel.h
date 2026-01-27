// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QSet>

#include "basetreemodel.h"

class DeviceInfo;

/**
 * @brief Tree model that organizes devices by their physical connection hierarchy.
 *
 * This model displays devices in a tree structure that reflects the actual
 * hardware connection topology. Devices are shown as children of their
 * parent bus or controller devices.
 */
class DevicesByConnectionModel : public BaseTreeModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DevicesByConnectionModel.
     * @param parent Optional parent QObject.
     */
    explicit DevicesByConnectionModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void buildTree();
    void collectAncestorSyspaths(const QSet<QString> &deviceSyspaths,
                                 QSet<QString> &allSyspaths) const;
    QString getNodeName(const DeviceInfo &info, QString *rawName = nullptr) const;

    Node *hostnameItem;
};
