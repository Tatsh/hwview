#pragma once

#include "models/basetreemodel.h"

/**
 * @brief Model for displaying exported drivers with their associated devices.
 */
class ViewerDriversByDeviceModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit ViewerDriversByDeviceModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void buildTree();

    Node *hostnameItem;
};
