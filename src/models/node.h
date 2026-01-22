#pragma once

#include <QIcon>
#include <QList>
#include <QPixmap>
#include <QString>
#include <QVariant>

enum NodeType {
    Device,
    Label,
};

class Node {
public:
    Node() : parentItem_(nullptr), type_(Label), row_(0) {
    }
    explicit Node(const QList<QVariant> &data, Node *parentItem = nullptr, NodeType type = Label);
    ~Node();

    void appendChild(Node *);
    Node *child(int);
    int childCount() const;
    void sortChildren();
    int columnCount() const;
    QVariant data(int) const;
    QIcon icon() const;
    Node *parentItem();
    int row() const;
    void setIcon(const QIcon &icon);
    void setType(NodeType);
    NodeType type() const;

    // Device-specific data
    QString syspath() const;
    void setSyspath(const QString &syspath);

    // Hidden device flag
    bool isHidden() const;
    void setIsHidden(bool hidden);

    // Get disabled pixmap (cached for performance)
    QPixmap disabledPixmap(int size = 16) const;

private:
    QList<QVariant> itemData;
    Node *parentItem_;
    NodeType type_;
    QList<Node *> childItems;
    QIcon icon_;
    QString syspath_;
    bool isHidden_ = false;
    int row_ = 0;                    // Cached row index for O(1) lookup
    mutable QPixmap disabledPixmap_; // Cached disabled pixmap
    mutable bool disabledPixmapCached_ = false;
};
