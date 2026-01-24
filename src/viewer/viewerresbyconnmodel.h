#pragma once

#include "models/basetreemodel.h"

/**
 * @brief Model for displaying exported resources organized by connection hierarchy.
 */
class ViewerResourcesByConnectionModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit ViewerResourcesByConnectionModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    bool shouldShowIcons() const override;

private:
    void buildTree();
    void addDma();
    void addIoPorts();
    void addIrq();
    void addMemory();
    void parseHierarchicalResource(const QJsonArray &lines, Node *categoryNode, const QIcon &icon);

    Node *hostnameItem;
    Node *dmaItem;
    Node *ioItem;
    Node *irqItem;
    Node *memoryItem;
};
