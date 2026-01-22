#pragma once

#include "basetreemodel.h"

class DevicesByDriverModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit DevicesByDriverModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void buildTree();

    Node *hostnameItem;
};
