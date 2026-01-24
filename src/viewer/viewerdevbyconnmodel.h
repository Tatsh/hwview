#pragma once

#include "models/basetreemodel.h"

#include <QHash>

/**
 * @brief Model for displaying exported devices organized by connection hierarchy.
 */
class ViewerDevicesByConnectionModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit ViewerDevicesByConnectionModel(QObject *parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

protected:
    QVariant decorationData(Node *item, int column) const override;

private:
    void buildTree();

    Node *hostnameItem;
    QHash<QString, Node *> nodesBySyspath_;
};
