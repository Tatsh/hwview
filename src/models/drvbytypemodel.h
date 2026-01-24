#pragma once

#include "basetreemodel.h"

/**
 * @brief Tree model that organizes drivers by device category/type.
 *
 * This model displays drivers grouped under category nodes based on
 * the type of devices they support. Each category contains the drivers
 * that handle devices of that type.
 */
class DriversByTypeModel : public BaseTreeModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a DriversByTypeModel.
     * @param parent Optional parent QObject.
     */
    explicit DriversByTypeModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void buildTree();
    void finalizeCategory(Node *&category);

    Node *hostnameItem;
};
