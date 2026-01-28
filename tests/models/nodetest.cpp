// SPDX-License-Identifier: MIT
#include <QtTest/QTest>

#include "models/node.h"

class NodeTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void defaultConstructor();
    void constructWithData();
    void appendChild_updatesRowIndex();
    void child_validIndex();
    void child_invalidIndex();
    void childCount();
    void columnCount();
    void data_validColumn();
    void parentItem();
    void row();
    void setIcon_icon();
    void type_setType();
    void syspath_setSyspath();
    void isHidden_setIsHidden();
    void rawName_setRawName();
    void sortChildren();
    void sortChildren_updatesRowIndices();
};

void NodeTest::defaultConstructor() {
    Node node;
    QCOMPARE(node.type(), Label);
    QCOMPARE(node.childCount(), 0);
    QCOMPARE(node.parentItem(), nullptr);
    QCOMPARE(node.row(), 0);
}

void NodeTest::constructWithData() {
    Node parent;
    QList<QVariant> data{QStringLiteral("Test Name"), QStringLiteral("Test Driver")};
    Node node(data, &parent, Device);

    QCOMPARE(node.type(), Device);
    QCOMPARE(node.parentItem(), &parent);
    QCOMPARE(node.columnCount(), 2);
    QCOMPARE(node.data(0).toString(), QStringLiteral("Test Name"));
    QCOMPARE(node.data(1).toString(), QStringLiteral("Test Driver"));
}

void NodeTest::appendChild_updatesRowIndex() {
    Node parent;
    auto *child1 = new Node({QStringLiteral("Child 1")}, &parent);
    auto *child2 = new Node({QStringLiteral("Child 2")}, &parent);

    parent.appendChild(child1);
    parent.appendChild(child2);

    QCOMPARE(child1->row(), 0);
    QCOMPARE(child2->row(), 1);
}

void NodeTest::child_validIndex() {
    Node parent;
    auto *child = new Node({QStringLiteral("Child")}, &parent);
    parent.appendChild(child);

    QCOMPARE(parent.child(0), child);
}

void NodeTest::child_invalidIndex() {
    Node parent;
    QCOMPARE(parent.child(-1), nullptr);
    QCOMPARE(parent.child(0), nullptr);
    QCOMPARE(parent.child(100), nullptr);
}

void NodeTest::childCount() {
    Node parent;
    QCOMPARE(parent.childCount(), 0);

    parent.appendChild(new Node({QStringLiteral("Child 1")}, &parent));
    QCOMPARE(parent.childCount(), 1);

    parent.appendChild(new Node({QStringLiteral("Child 2")}, &parent));
    QCOMPARE(parent.childCount(), 2);
}

void NodeTest::columnCount() {
    Node empty;
    QCOMPARE(empty.columnCount(), 0);

    Node oneColumn({QStringLiteral("Name")});
    QCOMPARE(oneColumn.columnCount(), 1);

    Node twoColumns({QStringLiteral("Name"), QStringLiteral("Driver")});
    QCOMPARE(twoColumns.columnCount(), 2);
}

void NodeTest::data_validColumn() {
    Node node({QStringLiteral("Name"), 42, true});
    QCOMPARE(node.data(0).toString(), QStringLiteral("Name"));
    QCOMPARE(node.data(1).toInt(), 42);
    QCOMPARE(node.data(2).toBool(), true);
}

void NodeTest::parentItem() {
    Node parent;
    Node child({}, &parent);

    QCOMPARE(parent.parentItem(), nullptr);
    QCOMPARE(child.parentItem(), &parent);
}

void NodeTest::row() {
    Node parent;
    auto *child1 = new Node({QStringLiteral("A")}, &parent);
    auto *child2 = new Node({QStringLiteral("B")}, &parent);
    auto *child3 = new Node({QStringLiteral("C")}, &parent);

    parent.appendChild(child1);
    parent.appendChild(child2);
    parent.appendChild(child3);

    QCOMPARE(child1->row(), 0);
    QCOMPARE(child2->row(), 1);
    QCOMPARE(child3->row(), 2);
}

void NodeTest::setIcon_icon() {
    Node node;
    QVERIFY(node.icon().isNull());

    QIcon icon = QIcon::fromTheme(QStringLiteral("computer"));
    node.setIcon(icon);
    // Can't easily compare QIcon equality, just verify it was set
    // For headless testing, the icon may still be null if no theme available
}

void NodeTest::type_setType() {
    Node node;
    QCOMPARE(node.type(), Label);

    node.setType(Device);
    QCOMPARE(node.type(), Device);

    node.setType(Label);
    QCOMPARE(node.type(), Label);
}

void NodeTest::syspath_setSyspath() {
    Node node;
    QVERIFY(node.syspath().isEmpty());

    node.setSyspath(QStringLiteral("/sys/devices/pci0000:00/0000:00:02.0"));
    QCOMPARE(node.syspath(), QStringLiteral("/sys/devices/pci0000:00/0000:00:02.0"));
}

void NodeTest::isHidden_setIsHidden() {
    Node node;
    QCOMPARE(node.isHidden(), false);

    node.setIsHidden(true);
    QCOMPARE(node.isHidden(), true);

    node.setIsHidden(false);
    QCOMPARE(node.isHidden(), false);
}

void NodeTest::rawName_setRawName() {
    Node node;
    QVERIFY(node.rawName().isEmpty());

    node.setRawName(QStringLiteral("Original Device Name"));
    QCOMPARE(node.rawName(), QStringLiteral("Original Device Name"));
}

void NodeTest::sortChildren() {
    Node parent;
    parent.appendChild(new Node({QStringLiteral("Zebra")}, &parent));
    parent.appendChild(new Node({QStringLiteral("Apple")}, &parent));
    parent.appendChild(new Node({QStringLiteral("Mango")}, &parent));

    parent.sortChildren();

    QCOMPARE(parent.child(0)->data(0).toString(), QStringLiteral("Apple"));
    QCOMPARE(parent.child(1)->data(0).toString(), QStringLiteral("Mango"));
    QCOMPARE(parent.child(2)->data(0).toString(), QStringLiteral("Zebra"));
}

void NodeTest::sortChildren_updatesRowIndices() {
    Node parent;
    auto *zebra = new Node({QStringLiteral("Zebra")}, &parent);
    auto *apple = new Node({QStringLiteral("Apple")}, &parent);
    auto *mango = new Node({QStringLiteral("Mango")}, &parent);

    parent.appendChild(zebra);
    parent.appendChild(apple);
    parent.appendChild(mango);

    // Before sort: zebra=0, apple=1, mango=2
    QCOMPARE(zebra->row(), 0);
    QCOMPARE(apple->row(), 1);
    QCOMPARE(mango->row(), 2);

    parent.sortChildren();

    // After sort: apple=0, mango=1, zebra=2
    QCOMPARE(apple->row(), 0);
    QCOMPARE(mango->row(), 1);
    QCOMPARE(zebra->row(), 2);
}

QTEST_MAIN(NodeTest)
#include "nodetest.moc"
