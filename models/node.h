#ifndef NODE_H
#define NODE_H

#include <QtCore/QVariant>
#include <QtCore/QVector>
#include <QtGui/QIcon>

enum NodeType {
    Device,
    Label,
};

class Node {
public:
    Node() = default;
    explicit Node(const QVector<QVariant> &data,
                  Node *parentItem = nullptr,
                  NodeType type = Label);
    ~Node();

    void appendChild(Node *);
    Node *child(int);
    int childCount() const;
    int columnCount() const;
    QVariant data(int) const;
    QIcon icon() const;
    Node *parentItem();
    int row() const;
    void setIcon(QIcon);
    void setIconFromTheme(const QString &);
    void setType(NodeType);
    NodeType type();

private:
    Node *parentItem_;
    QVector<Node *> childItems;
    QIcon icon_;
    QVector<QVariant> itemData;
    NodeType type_;
};

#endif // NODE_H
