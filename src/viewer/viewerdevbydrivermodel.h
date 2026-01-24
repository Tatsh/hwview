#pragma once

#include "models/basetreemodel.h"

/**
 * @brief Model for displaying exported devices organized by driver.
 */
class ViewerDevicesByDriverModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit ViewerDevicesByDriverModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void buildTree();

    Node *hostnameItem;
};
