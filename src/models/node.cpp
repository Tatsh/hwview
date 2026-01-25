#include <algorithm>

#include "models/node.h"

Node::Node(const QList<QVariant> &data, Node *parent, NodeType nodeType)
    : itemData(data), parentItem_(parent), type_(nodeType) {
}

Node::~Node() {
    qDeleteAll(childItems);
}

void Node::appendChild(Node *item) {
    item->row_ = static_cast<int>(childItems.size());
    childItems.append(item);
}

Node *Node::child(int row) {
    if (row < 0 || row >= childItems.size()) {
        return nullptr;
    }
    return childItems.at(row);
}

int Node::childCount() const {
    return static_cast<int>(childItems.count());
}

int Node::columnCount() const {
    return static_cast<int>(itemData.count());
}

QVariant Node::data(int column) const {
    Q_ASSERT(column >= 0 && column < itemData.size());
    return itemData.at(column);
}

Node *Node::parentItem() {
    return parentItem_;
}

int Node::row() const {
    return row_;
}

QIcon Node::icon() const {
    return icon_;
}

void Node::setIcon(const QIcon &icon) {
    icon_ = icon;
}

void Node::setType(NodeType type) {
    type_ = type;
}

NodeType Node::type() const {
    return type_;
}

QString Node::syspath() const {
    return syspath_;
}

void Node::setSyspath(const QString &syspath) {
    syspath_ = syspath;
}

bool Node::isHidden() const {
    return isHidden_;
}

void Node::setIsHidden(bool hidden) {
    isHidden_ = hidden;
}

QString Node::rawName() const {
    return rawName_;
}

void Node::setRawName(const QString &name) {
    rawName_ = name;
}

void Node::sortChildren() {
    std::sort(childItems.begin(), childItems.end(), [](const Node *a, const Node *b) {
        // Sort alphabetically by the first column (name)
        QString nameA = a->data(0).toString().toLower();
        QString nameB = b->data(0).toString().toLower();
        return nameA < nameB;
    });
    // Update row indices after sorting
    for (int i = 0; i < childItems.size(); ++i) {
        childItems[i]->row_ = i;
    }
}

QPixmap Node::disabledPixmap(int size) const {
    if (!disabledPixmapCached_) {
        disabledPixmap_ = icon_.pixmap(size, QIcon::Disabled);
        disabledPixmapCached_ = true;
    }
    return disabledPixmap_;
}
