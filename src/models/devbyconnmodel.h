#pragma once

#include <QSet>

#include "basetreemodel.h"

class DeviceInfo;

class DevicesByConnectionModel : public BaseTreeModel {
    Q_OBJECT

public:
    explicit DevicesByConnectionModel(QObject *parent = nullptr);
    QVariant
    headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void buildTree();
    void collectAncestorSyspaths(const QSet<QString> &deviceSyspaths,
                                 QSet<QString> &allSyspaths) const;
    QString getNodeName(const DeviceInfo &info) const;

    Node *hostnameItem;
};
