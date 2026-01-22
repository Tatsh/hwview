#pragma once

#include "basetreemodel.h"

class DriversByTypeModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit DriversByTypeModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void buildTree();
    void finalizeCategory(Node *&category);

    Node *hostnameItem;
};
