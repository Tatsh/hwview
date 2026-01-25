#pragma once

#include <QStack>

#include "basetreemodel.h"

/**
 * @brief Tree model that organizes system resources by connection hierarchy.
 *
 * This model displays system resources (IRQs, I/O ports, DMA channels,
 * memory ranges) in a hierarchical structure that reflects how they are
 * allocated to devices in the system.
 */
class ResourcesByConnectionModel : public BaseTreeModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a ResourcesByConnectionModel.
     * @param parent Optional parent QObject.
     */
    explicit ResourcesByConnectionModel(QObject *parent = nullptr);
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
    void buildHierarchicalResource(Node *categoryNode, const QIcon &itemIcon, int indentLevel,
                                   const QString &rangeStart, const QString &rangeEnd,
                                   const QString &name, QStack<QPair<int, Node *>> &nodeStack);

    Node *hostnameItem;
    Node *dmaItem;
    Node *ioItem;
    Node *irqItem;
    Node *memoryItem;
};
