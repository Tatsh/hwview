#pragma once

#include <QAbstractItemModel>

#include "node.h"

// Base class for tree models that use Node as the underlying data structure.
// Provides common implementations of QAbstractItemModel methods.
class BaseTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit BaseTreeModel(QObject *parent = nullptr);
    ~BaseTreeModel() override;

    // QAbstractItemModel interface
    QModelIndex
    index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    // Returns the root item. Subclasses must set this in their constructor.
    Node *rootItem() const;
    void setRootItem(Node *root);

    // Override in subclasses to customize icon display behavior
    // Default returns true (always show icons)
    virtual bool shouldShowIcons() const;

    // Override in subclasses to customize decoration role
    // Default checks column 0 only and respects shouldShowIcons()
    virtual QVariant decorationData(Node *item, int column) const;

private:
    Node *rootItem_ = nullptr;
};
