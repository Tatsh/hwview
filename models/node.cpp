#include "node.h"

Node::Node(const QVector<QVariant> &data, Node *parent)
    : itemData(data), parentItem_(parent) {
}

Node::~Node() {
    qDeleteAll(childItems);
}

void Node::appendChild(Node *item) {
    childItems.append(item);
}

Node *Node::child(int row) {
    if (row < 0 || row >= childItems.size()) {
        return nullptr;
    }
    return childItems.at(row);
}

int Node::childCount() const {
    return childItems.count();
}

int Node::columnCount() const {
    return itemData.count();
}

QVariant Node::data(int column) const {
    if (column < 0 || column >= itemData.size()) {
        return QVariant();
    }
    return itemData.at(column);
}

Node *Node::parentItem() {
    return parentItem_;
}

int Node::row() const {
    if (parentItem_) {
        return parentItem_->childItems.indexOf(const_cast<Node *>(this));
    }
    return 0;
}

QIcon Node::icon() const {
    return icon_;
}

void Node::setIcon(QIcon icon) {
    icon_ = icon;
}
