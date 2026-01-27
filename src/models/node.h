// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>

/**
 * @brief Enumeration of node types in the device tree.
 */
enum NodeType {
    Device, ///< Represents an actual device.
    Label,  ///< Represents a category or label node.
};

/**
 * @brief Represents a node in the device tree model.
 *
 * This class is used as the underlying data structure for all tree models in the application. Each
 * node can have children, forming a hierarchical tree structure. Nodes can represent either devices
 * or category labels.
 */
class Node {
public:
    /**
     * @brief Constructs an empty label node with no parent.
     */
    Node() : parentItem_(nullptr), type_(Label), row_(0) {
    }

    /**
     * @brief Constructs a node with the specified data.
     * @param data List of column data for this node.
     * @param parentItem Parent node, or @c nullptr for root nodes.
     * @param type The type of this node (@c Device or @c Label).
     */
    explicit Node(const QList<QVariant> &data, Node *parentItem = nullptr, NodeType type = Label);
    ~Node();

    /**
     * @brief Appends a child node to this node.
     * @param child The child node to append. This node takes ownership.
     */
    void appendChild(Node *child);

    /**
     * @brief Returns the child at the specified row.
     * @param row The row index of the child.
     * @returns The child node, or @c nullptr if row is out of range.
     */
    Node *child(int row);

    /**
     * @brief Returns the number of children.
     * @returns The child count.
     */
    int childCount() const;

    /**
     * @brief Sorts children alphabetically by their first column data.
     */
    void sortChildren();

    /**
     * @brief Returns the number of data columns.
     * @returns The column count.
     */
    int columnCount() const;

    /**
     * @brief Returns the data for the specified column.
     * @param column The column index.
     * @returns The data as a @c QVariant.
     */
    QVariant data(int column) const;

    /**
     * @brief Returns the icon for this node.
     * @returns The node's icon.
     */
    QIcon icon() const;

    /**
     * @brief Returns the parent node.
     * @returns The parent node, or @c nullptr for root nodes.
     */
    Node *parentItem();

    /**
     * @brief Returns this node's row index within its parent.
     * @returns The row index.
     */
    int row() const;

    /**
     * @brief Sets the icon for this node.
     * @param icon The icon to set.
     */
    void setIcon(const QIcon &icon);

    /**
     * @brief Sets the type of this node.
     * @param type The node type to set.
     */
    void setType(NodeType type);

    /**
     * @brief Returns the type of this node.
     * @returns The node type.
     */
    NodeType type() const;

    /**
     * @brief Returns the system path for device nodes.
     * @returns The syspath string, or empty for non-device nodes.
     */
    QString syspath() const;

    /**
     * @brief Sets the system path for this node.
     * @param syspath The system path to set.
     */
    void setSyspath(const QString &syspath);

    /**
     * @brief Returns whether this node represents a hidden device.
     * @returns @c true if the device is hidden, @c false otherwise.
     */
    bool isHidden() const;

    /**
     * @brief Sets whether this node represents a hidden device.
     * @param hidden @c true to mark as hidden, @c false otherwise.
     */
    void setIsHidden(bool hidden);

    /**
     * @brief Returns the raw/original name before display name conversion.
     * @returns The raw name, or empty if not set.
     */
    QString rawName() const;

    /**
     * @brief Sets the raw/original name before display name conversion.
     * @param name The original name.
     */
    void setRawName(const QString &name);

    /**
     * @brief Returns a cached disabled (grayed out) pixmap of the icon.
     * @param size The desired icon size in pixels.
     * @returns The disabled pixmap.
     */
    QPixmap disabledPixmap(int size = 16) const;

private:
    QList<QVariant> itemData;
    Node *parentItem_;
    NodeType type_;
    QList<Node *> childItems;
    QIcon icon_;
    QString syspath_;
    QString rawName_;
    bool isHidden_ = false;
    int row_ = 0;
    mutable QPixmap disabledPixmap_;
    mutable bool disabledPixmapCached_ = false;
};
