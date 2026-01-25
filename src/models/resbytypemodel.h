#pragma once

#include "basetreemodel.h"

/**
 * @brief Tree model that organizes system resources by type.
 *
 * This model displays system resources (IRQs, I/O ports, DMA channels,
 * memory ranges) grouped by their type. Each resource type is a category
 * node containing the individual resource allocations.
 */
class ResourcesByTypeModel : public BaseTreeModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a ResourcesByTypeModel.
     * @param parent Optional parent QObject.
     */
    explicit ResourcesByTypeModel(QObject *parent = nullptr);
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    bool shouldShowIcons() const override;

private:
    void buildTree();
    void addDma();
    void addIoPorts();
    void addIrq();
    void addMemory();

    Node *hostnameItem;
    Node *dmaItem;
    Node *ioItem;
    Node *irqItem;
    Node *memoryItem;
};
