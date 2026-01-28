// SPDX-License-Identifier: MIT
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtTest/QTest>

#include "deviceinfo.h"

class ImportedDeviceInfoTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void loadFromExportFile();
    void propertyValue_existing();
    void propertyValue_nonExisting();
    void propertiesObject_containsData();
    void driverInfoObject_containsData();
    void resourcesArray_containsData();
    void categoryPreserved();
    void hiddenFlagPreserved();
    void allCategories_parseCorrectly();
    void multipleDevices_parseIndependently();
};

void ImportedDeviceInfoTest::loadFromExportFile() {
    QString filePath =
        QStringLiteral(HWVIEW_TEST_DATA_DIR) + QStringLiteral("/sample-export.dmexport");
    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    QCOMPARE(error.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());

    auto root = doc.object();
    QVERIFY(root.contains(QStringLiteral("devices")));

    auto devicesArray = root[QStringLiteral("devices")].toArray();
    QVERIFY(!devicesArray.isEmpty());

    // Load first device
    DeviceInfo info(devicesArray[0].toObject());

    QVERIFY(info.isImported());
    QVERIFY(!info.syspath().isEmpty());
    QVERIFY(!info.name().isEmpty());
}

void ImportedDeviceInfoTest::propertyValue_existing() {
    QJsonObject props;
    props[QStringLiteral("PCI_ID")] = QStringLiteral("8086:3E92");
    props[QStringLiteral("DRIVER")] = QStringLiteral("i915");

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);
    json[QStringLiteral("properties")] = props;

    DeviceInfo info(json);

    QCOMPARE(info.propertyValue("PCI_ID"), QStringLiteral("8086:3E92"));
    QCOMPARE(info.propertyValue("DRIVER"), QStringLiteral("i915"));
}

void ImportedDeviceInfoTest::propertyValue_nonExisting() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    DeviceInfo info(json);

    QVERIFY(info.propertyValue("NONEXISTENT_PROPERTY").isEmpty());
}

void ImportedDeviceInfoTest::propertiesObject_containsData() {
    QJsonObject props;
    props[QStringLiteral("KEY1")] = QStringLiteral("VALUE1");
    props[QStringLiteral("KEY2")] = QStringLiteral("VALUE2");

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::SystemDevices);
    json[QStringLiteral("properties")] = props;

    DeviceInfo info(json);

    QCOMPARE(info.properties().size(), 2);
    QCOMPARE(info.properties()[QStringLiteral("KEY1")].toString(), QStringLiteral("VALUE1"));
}

void ImportedDeviceInfoTest::driverInfoObject_containsData() {
    QJsonObject driverInfo;
    driverInfo[QStringLiteral("hasDriver")] = true;
    driverInfo[QStringLiteral("name")] = QStringLiteral("test_driver");
    driverInfo[QStringLiteral("version")] = QStringLiteral("1.0.0");

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::SystemDevices);
    json[QStringLiteral("driverInfo")] = driverInfo;

    DeviceInfo info(json);

    QVERIFY(info.driverInfo()[QStringLiteral("hasDriver")].toBool());
    QCOMPARE(info.driverInfo()[QStringLiteral("name")].toString(), QStringLiteral("test_driver"));
}

void ImportedDeviceInfoTest::resourcesArray_containsData() {
    QJsonArray resources;

    QJsonObject res1;
    res1[QStringLiteral("type")] = QStringLiteral("Memory");
    res1[QStringLiteral("displayValue")] = QStringLiteral("0xA0000000");
    resources.append(res1);

    QJsonObject res2;
    res2[QStringLiteral("type")] = QStringLiteral("IRQ");
    res2[QStringLiteral("displayValue")] = QStringLiteral("16");
    resources.append(res2);

    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);
    json[QStringLiteral("resources")] = resources;

    DeviceInfo info(json);

    QCOMPARE(info.resources().size(), 2);
    QCOMPARE(info.resources()[0].toObject()[QStringLiteral("type")].toString(),
             QStringLiteral("Memory"));
    QCOMPARE(info.resources()[1].toObject()[QStringLiteral("type")].toString(),
             QStringLiteral("IRQ"));
}

void ImportedDeviceInfoTest::categoryPreserved() {
    QJsonObject json;
    json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
    json[QStringLiteral("name")] = QStringLiteral("Test Device");
    json[QStringLiteral("category")] = static_cast<int>(DeviceCategory::Batteries);

    DeviceInfo info(json);

    QCOMPARE(info.category(), DeviceCategory::Batteries);
}

void ImportedDeviceInfoTest::hiddenFlagPreserved() {
    QJsonObject jsonHidden;
    jsonHidden[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/hidden");
    jsonHidden[QStringLiteral("name")] = QStringLiteral("Hidden");
    jsonHidden[QStringLiteral("isHidden")] = true;
    jsonHidden[QStringLiteral("category")] = static_cast<int>(DeviceCategory::SoftwareDevices);

    DeviceInfo hidden(jsonHidden);
    QVERIFY(hidden.isHidden());

    QJsonObject jsonVisible;
    jsonVisible[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/visible");
    jsonVisible[QStringLiteral("name")] = QStringLiteral("Visible");
    jsonVisible[QStringLiteral("isHidden")] = false;
    jsonVisible[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    DeviceInfo visible(jsonVisible);
    QVERIFY(!visible.isHidden());
}

void ImportedDeviceInfoTest::allCategories_parseCorrectly() {
    // Test that all category enum values are correctly parsed
    QList<DeviceCategory> categories = {
        DeviceCategory::Unknown,
        DeviceCategory::AudioInputsAndOutputs,
        DeviceCategory::Batteries,
        DeviceCategory::Computer,
        DeviceCategory::DiskDrives,
        DeviceCategory::DisplayAdapters,
        DeviceCategory::DvdCdromDrives,
        DeviceCategory::HumanInterfaceDevices,
        DeviceCategory::Keyboards,
        DeviceCategory::MiceAndOtherPointingDevices,
        DeviceCategory::NetworkAdapters,
        DeviceCategory::SoftwareDevices,
        DeviceCategory::SoundVideoAndGameControllers,
        DeviceCategory::StorageControllers,
        DeviceCategory::StorageVolumes,
        DeviceCategory::SystemDevices,
        DeviceCategory::UniversalSerialBusControllers,
    };

    for (auto category : categories) {
        QJsonObject json;
        json[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/test");
        json[QStringLiteral("name")] = QStringLiteral("Test");
        json[QStringLiteral("category")] = static_cast<int>(category);

        DeviceInfo info(json);
        QCOMPARE(info.category(), category);
    }
}

void ImportedDeviceInfoTest::multipleDevices_parseIndependently() {
    QJsonObject json1;
    json1[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/device1");
    json1[QStringLiteral("name")] = QStringLiteral("Device 1");
    json1[QStringLiteral("driver")] = QStringLiteral("driver1");
    json1[QStringLiteral("category")] = static_cast<int>(DeviceCategory::DisplayAdapters);

    QJsonObject json2;
    json2[QStringLiteral("syspath")] = QStringLiteral("/sys/devices/device2");
    json2[QStringLiteral("name")] = QStringLiteral("Device 2");
    json2[QStringLiteral("driver")] = QStringLiteral("driver2");
    json2[QStringLiteral("category")] = static_cast<int>(DeviceCategory::NetworkAdapters);

    DeviceInfo info1(json1);
    DeviceInfo info2(json2);

    // Verify they are independent
    QCOMPARE(info1.name(), QStringLiteral("Device 1"));
    QCOMPARE(info2.name(), QStringLiteral("Device 2"));
    QCOMPARE(info1.driver(), QStringLiteral("driver1"));
    QCOMPARE(info2.driver(), QStringLiteral("driver2"));
    QCOMPARE(info1.category(), DeviceCategory::DisplayAdapters);
    QCOMPARE(info2.category(), DeviceCategory::NetworkAdapters);
}

QTEST_MAIN(ImportedDeviceInfoTest)
#include "importeddeviceinfotest.moc"
