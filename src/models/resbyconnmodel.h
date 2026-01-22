#pragma once

#include "basetreemodel.h"

class ResourcesByConnectionModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit ResourcesByConnectionModel(QObject *parent = nullptr);
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    bool shouldShowIcons() const override;

private:
    void buildTree();
#ifdef Q_OS_LINUX
    void addDma();
    void addIoPorts();
    void addIrq();
    void addMemory();
    void parseHierarchicalResource(const QString &filePath, Node *categoryNode);
#endif

    Node *hostnameItem;
    Node *dmaItem;
    Node *ioItem;
    Node *irqItem;
    Node *memoryItem;
};
