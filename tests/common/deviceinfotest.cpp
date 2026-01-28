// SPDX-License-Identifier: MIT
#include <QtCore/QJsonObject>
#include <QtTest/QTest>

#include "deviceinfo.h"

class DeviceInfoTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void constructFromJson_basicFields();
    void constructFromJson_pciInfo();
    void constructFromJson_idsInfo();
    void constructFromJson_category();
    void constructFromJson_isImported();
    void constructFromJson_isHidden();
    void copyConstructor();
    void copyAssignment();
    void moveConstructor();
    void moveAssignment();
    void propertyValue_imported();
    void properties_imported();
    void properties_nonImported();
    void driverInfo_imported();
    void driverInfo_nonImported();
    void resources_imported();
    void resources_nonImported();
    void isValidForDisplay_validCategory();
    void isValidForDisplay_unknownCategory();
    void emptyDeviceInfo_handlesNullptr();
};

void DeviceInfoTest::constructFromJson_basicFields() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("driver")] = QStringLiteral("test_driver");
    json[QStringLiteral("subsystem")] = QStringLiteral("pci");
    json[QStringLiteral("devnode")] = QStringLiteral("/dev/test0");
    json[QStringLiteral("parentSyspath")] = QStringLiteral("/sys/devices/parent");
    json[QStringLiteral("devPath")] = QStringLiteral("/devices/test");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    DeviceInfo info(json);

    QCOMPARE(info.syspath(), QStringLiteral("/sys/devices/test"));
    QCOMPARE(info.name(), QStringLiteral("Test Device"));
    QCOMPARE(info.driver(), QStringLiteral("test_driver"));
    QCOMPARE(info.subsystem(), QStringLiteral("pci"));
    QCOMPARE(info.devnode(), QStringLiteral("/dev/test0"));
    QCOMPARE(info.parentSyspath(), QStringLiteral("/sys/devices/parent"));
    QCOMPARE(info.devPath(), QStringLiteral("/devices/test"));
}

void DeviceInfoTest::constructFromJson_pciInfo() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test PCI Device");
    json[QStringLiteral("pciClass")] = QStringLiteral("Display controller");
    json[QStringLiteral("pciSubclass")] = QStringLiteral("VGA compatible controller");
    json[QStringLiteral("pciInterface")] = QStringLiteral("VGA");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    DeviceInfo info(json);

    QCOMPARE(info.pciClass(), QStringLiteral("Display controller"));
    QCOMPARE(info.pciSubclass(), QStringLiteral("VGA compatible controller"));
    QCOMPARE(info.pciInterface(), QStringLiteral("VGA"));
}

void DeviceInfoTest::constructFromJson_idsInfo() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("idCdrom")] = QStringLiteral("1");
    json[QStringLiteral("idDevType")] = QStringLiteral("disk");
    json[QStringLiteral("idInputKeyboard")] = QStringLiteral("1");
    json[QStringLiteral("idInputMouse")] = QStringLiteral("0");
    json[QStringLiteral("idType")] = QStringLiteral("disk");
    json[QStringLiteral("idModelFromDatabase")] = QStringLiteral("Test Model");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DiskDrives);

    DeviceInfo info(json);

    QCOMPARE(info.idCdrom(), QStringLiteral("1"));
    QCOMPARE(info.devType(), QStringLiteral("disk"));
    QCOMPARE(info.idInputKeyboard(), QStringLiteral("1"));
    QCOMPARE(info.idInputMouse(), QStringLiteral("0"));
    QCOMPARE(info.idType(), QStringLiteral("disk"));
    QCOMPARE(info.idModelFromDatabase(), QStringLiteral("Test Model"));
}

void DeviceInfoTest::constructFromJson_category() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::NetworkAdapters);

    DeviceInfo info(json);

    QCOMPARE(info.category(), DeviceCategory::NetworkAdapters);
}

void DeviceInfoTest::constructFromJson_isImported() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::SystemDevices);

    DeviceInfo info(json);

    QVERIFY(info.isImported());
}

void DeviceInfoTest::constructFromJson_isHidden() {
    QJsonObject jsonHidden;
    jsonHidden[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/hidden");
    jsonHidden[QStringLiteral("name")] = QStringLiteral("Hidden Device");
    jsonHidden[QStringLiteral("isHidden")] = true;
    jsonHidden[QStringLiteral("category")] = static_cast<int>(DeviceCategory::SoftwareDevices);

    DeviceInfo hiddenInfo(jsonHidden);
    QVERIFY(hiddenInfo.isHidden());

    QJsonObject jsonVisible;
    jsonVisible[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/visible");
    jsonVisible[QStringLiteral("name")] = QStringLiteral("Visible Device");
    jsonVisible[QStringLiteral("isHidden")] = false;
    jsonVisible[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    DeviceInfo visibleInfo(jsonVisible);
    QVERIFY(!visibleInfo.isHidden());
}

void DeviceInfoTest::copyConstructor() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/original");
    json[QStringLiteral("name")] = QStringLiteral("Original Device");
    json[QStringLiteral("driver")] = QStringLiteral("original_driver");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    DeviceInfo original(json);
    DeviceInfo copy(original);

    QCOMPARE(copy.syspath(), original.syspath());
    QCOMPARE(copy.name(), original.name());
    QCOMPARE(copy.driver(), original.driver());
    QCOMPARE(copy.category(), original.category());
    QCOMPARE(copy.isImported(), original.isImported());
}

void DeviceInfoTest::copyAssignment() {
    QJsonObject json1;
    json1[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/first");
    json1[QStringLiteral("name")] = QStringLiteral("First Device");
    json1[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    QJsonObject json2;
    json2[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/second");
    json2[QStringLiteral("name")] = QStringLiteral("Second Device");
    json2[QStringLiteral("category")] = static_cast<int>(DeviceCategory::NetworkAdapters);

    DeviceInfo first(json1);
    DeviceInfo second(json2);

    first = second;

    QCOMPARE(first.syspath(), QStringLiteral("/sys/devices/second"));
    QCOMPARE(first.name(), QStringLiteral("Second Device"));
    QCOMPARE(first.category(), DeviceCategory::NetworkAdapters);
}

void DeviceInfoTest::moveConstructor() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/movable");
    json[QStringLiteral("name")] = QStringLiteral("Movable Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::Keyboards);

    DeviceInfo original(json);
    DeviceInfo moved(std::move(original));

    QCOMPARE(moved.syspath(), QStringLiteral("/sys/devices/movable"));
    QCOMPARE(moved.name(), QStringLiteral("Movable Device"));
    QCOMPARE(moved.category(), DeviceCategory::Keyboards);
}

void DeviceInfoTest::moveAssignment() {
    QJsonObject json1;
    json1[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/first");
    json1[QStringLiteral("name")] = QStringLiteral("First");
    json1[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    QJsonObject json2;
    json2[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/second");
    json2[QStringLiteral("name")] = QStringLiteral("Second");
    json2[QStringLiteral("category")] = static_cast<int>(DeviceCategory::Batteries);

    DeviceInfo first(json1);
    DeviceInfo second(json2);

    first = std::move(second);

    QCOMPARE(first.syspath(), QStringLiteral("/sys/devices/second"));
    QCOMPARE(first.name(), QStringLiteral("Second"));
    QCOMPARE(first.category(), DeviceCategory::Batteries);
}

void DeviceInfoTest::propertyValue_imported() {
    QJsonObject props;
    props[QStringLiteral("PCI_ID")] = QStringLiteral("8086:3E92");
    props[QStringLiteral("DRIVER")] = QStringLiteral("i915");
    props[QStringLiteral("SUBSYSTEM")] = QStringLiteral("pci");

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);
    json[QStringLiteral("properties")] = props;

    DeviceInfo info(json);

    QCOMPARE(info.propertyValue("PCI_ID"), QStringLiteral("8086:3E92"));
    QCOMPARE(info.propertyValue("DRIVER"), QStringLiteral("i915"));
    QCOMPARE(info.propertyValue("SUBSYSTEM"), QStringLiteral("pci"));
    QVERIFY(info.propertyValue("NONEXISTENT").isEmpty());
}

void DeviceInfoTest::properties_imported() {
    QJsonObject props;
    props[QStringLiteral("PCI_ID")] = QStringLiteral("8086:3E92");
    props[QStringLiteral("DRIVER")] = QStringLiteral("i915");

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);
    json[QStringLiteral("properties")] = props;

    DeviceInfo info(json);

    QVERIFY(!info.properties().isEmpty());
    QCOMPARE(info.properties()[QStringLiteral("PCI_ID")].toString(), QStringLiteral("8086:3E92"));
}

void DeviceInfoTest::properties_nonImported() {
    // Non-imported device (created with nullptr) should return empty properties
    DeviceInfo info(static_cast<DeviceInfoPrivate *>(nullptr));
    QVERIFY(info.properties().isEmpty());
}

void DeviceInfoTest::driverInfo_imported() {
    QJsonObject driverInfo;
    driverInfo[QStringLiteral("hasDriver")] = true;
    driverInfo[QStringLiteral("name")] = QStringLiteral("i915");
    driverInfo[QStringLiteral("description")] = QStringLiteral("Intel Graphics Driver");

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);
    json[QStringLiteral("driverInfo")] = driverInfo;

    DeviceInfo info(json);

    QVERIFY(!info.driverInfo().isEmpty());
    QCOMPARE(info.driverInfo()[QStringLiteral("name")].toString(), QStringLiteral("i915"));
}

void DeviceInfoTest::driverInfo_nonImported() {
    // Non-imported device should return empty driverInfo
    DeviceInfo info(static_cast<DeviceInfoPrivate *>(nullptr));
    QVERIFY(info.driverInfo().isEmpty());
}

void DeviceInfoTest::resources_imported() {
    QJsonArray resources;
    QJsonObject resource;
    resource[QStringLiteral("type")] = QStringLiteral("Memory");
    resource[QStringLiteral("displayValue")] = QStringLiteral("0xA0000000 - 0xAFFFFFFF");
    resources.append(resource);

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);
    json[QStringLiteral("resources")] = resources;

    DeviceInfo info(json);

    QVERIFY(!info.resources().isEmpty());
    QCOMPARE(info.resources().size(), 1);
}

void DeviceInfoTest::resources_nonImported() {
    // Non-imported device should return empty resources
    DeviceInfo info(static_cast<DeviceInfoPrivate *>(nullptr));
    QVERIFY(info.resources().isEmpty());
}

void DeviceInfoTest::isValidForDisplay_validCategory() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    DeviceInfo info(json);

    QVERIFY(info.isValidForDisplay());
}

void DeviceInfoTest::isValidForDisplay_unknownCategory() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::Unknown);

    DeviceInfo info(json);

    QVERIFY(!info.isValidForDisplay());
}

void DeviceInfoTest::emptyDeviceInfo_handlesNullptr() {
    // Create a DeviceInfo with nullptr (edge case)
    DeviceInfo info(static_cast<DeviceInfoPrivate *>(nullptr));

    // Should handle gracefully and return empty/default values
    QVERIFY(info.name().isEmpty());
    QVERIFY(info.driver().isEmpty());
    QVERIFY(info.syspath().isEmpty());
    QVERIFY(info.isHidden()); // Default to hidden if no private impl
    QCOMPARE(info.category(), DeviceCategory::Unknown);
    QVERIFY(!info.isValidForDisplay());
}

QTEST_MAIN(DeviceInfoTest)
#include "deviceinfotest.moc"
