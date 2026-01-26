#include "models/basetreemodel.h"

BaseTreeModel::BaseTreeModel(QObject *parent) : QAbstractItemModel(parent) {
}

BaseTreeModel::~BaseTreeModel() {
    delete rootItem_;
}

Node *BaseTreeModel::rootItem() const {
    return rootItem_;
}

void BaseTreeModel::setRootItem(Node *root) {
    rootItem_ = root;
}

bool BaseTreeModel::shouldShowIcons() const {
    return true;
}

QVariant BaseTreeModel::decorationData(Node *item, int column) const {
    // Only show icons in the first column by default
    if (column != 0) {
        return {};
    }
    if (!shouldShowIcons()) {
        return {};
    }
    if (item->isHidden()) {
        // Return cached greyed-out pixmap for hidden devices
        return item->disabledPixmap(16);
    }
    return item->icon();
}

QModelIndex BaseTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return {};
    }
    Node *parentItem = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : rootItem_;
    if (Node *childItem = parentItem->child(row)) {
        return createIndex(row, column, childItem);
    }
    return {};
}

QModelIndex BaseTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) {
        return {};
    }
    auto *childItem = static_cast<Node *>(index.internalPointer());
    Node *parentItem = childItem->parentItem();
    if (parentItem == rootItem_) {
        return {};
    }
    return createIndex(parentItem->row(), 0, parentItem);
}

int BaseTreeModel::rowCount(const QModelIndex &parent) const {
    if (parent.column() > 0) {
        return 0;
    }
    Node *parentItem = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : rootItem_;
    return parentItem->childCount();
}

int BaseTreeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return static_cast<Node *>(parent.internalPointer())->columnCount();
    }
    return rootItem_->columnCount();
}

QVariant BaseTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    auto *item = static_cast<Node *>(index.internalPointer());
    switch (role) {
    case Qt::DecorationRole:
        return decorationData(item, index.column());
    case Qt::DisplayRole:
        return item->data(index.column());
    case Qt::ToolTipRole:
        // Show raw name as tooltip if it differs from display name
        if (index.column() == 0 && !item->rawName().isEmpty()) {
            auto displayName = item->data(0).toString();
            if (item->rawName() != displayName) {
                return item->rawName();
            }
        }
        return {};
    default:
        return {};
    }
}

Qt::ItemFlags BaseTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}
