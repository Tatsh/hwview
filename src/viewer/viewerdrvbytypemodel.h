#pragma once

#include "models/basetreemodel.h"

/**
 * @brief Model for displaying exported drivers organized by type/category.
 */
class ViewerDriversByTypeModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit ViewerDriversByTypeModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void buildTree();
    void finalizeCategory(Node *&category);

    Node *hostnameItem;
};
