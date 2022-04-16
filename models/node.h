#ifndef NODE_H
#define NODE_H

#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtGui/QIcon>

class Node {
public:
    Node() = default;
    explicit Node(const QVector<QVariant> &data, Node *parentItem = nullptr);
    ~Node();

    void appendChild(Node *child);
    Node *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    Node *parentItem();
    int row() const;
    QIcon icon() const;
    void setIcon(QIcon);

private:
    Node *parentItem_;
    QVector<Node *> childItems;
    QVector<QVariant> itemData;
    QIcon icon_;
};

#endif // NODE_H
