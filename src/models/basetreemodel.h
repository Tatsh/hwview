#pragma once

#include <QtCore/QAbstractItemModel>

#include "node.h"

/**
 * @brief Base class for tree models that use @c Node as the underlying data structure.
 *
 * This class provides common implementations of @c QAbstractItemModel methods for all device tree
 * models. Subclasses should create their tree structure in their constructor by setting the root
 * item with @c setRootItem().
 */
class BaseTreeModel : public QAbstractItemModel {
    Q_OBJECT

public:
    /**
     * @brief Constructs a @c BaseTreeModel.
     * @param parent Optional parent @c QObject.
     */
    explicit BaseTreeModel(QObject *parent = nullptr);
    ~BaseTreeModel() override;

    QModelIndex
    index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    /**
     * @brief Returns the root item of the tree.
     * @returns Pointer to the root @c Node.
     */
    Node *rootItem() const;

    /**
     * @brief Sets the root item of the tree.
     * @param root The root @c Node. This model takes ownership.
     */
    void setRootItem(Node *root);

    /**
     * @brief Returns whether icons should be displayed.
     *
     * Subclasses can override this to control icon display behavior. Default returns @c true.
     *
     * @returns @c true if icons should be shown, @c false otherwise.
     */
    virtual bool shouldShowIcons() const;

    /**
     * @brief Returns the decoration (icon) data for a node.
     * @param item The node to get decoration for.
     * @param column The column index.
     * @returns The icon as a @c QVariant, or empty @c QVariant if no icon.
     */
    virtual QVariant decorationData(Node *item, int column) const;

private:
    Node *rootItem_ = nullptr;
};
