// SPDX-License-Identifier: MIT
#include <QtTest/QTest>

#include "namemappings.h"

class NameMappingsTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void categoryNameFromGuid_known();
    void categoryNameFromGuid_unknown();
    void categoryNameFromGuid_caseInsensitive();
    void hidVendorName_known();
    void hidVendorName_unknown();
    void hidVendorName_caseInsensitive();
    void hidBusTypeName_knownTypes();
    void hidBusTypeName_unknownType();
    void softwareDeviceDisplayName_known();
    void softwareDeviceDisplayName_unknown();
    void acpiDeviceDisplayName_known();
    void acpiDeviceDisplayName_unknown();
    void acpiDeviceDisplayName_caseInsensitive();
    void vendorSupportUrl_known();
    void vendorSupportUrl_unknown();
    void clear_removesAllMappings();
    void loadFromFile_loadsMappings();
};

void NameMappingsTest::initTestCase() {
    // Clear and load test data
    NameMappings::instance().clear();
    QString testDataDir = QStringLiteral(HWVIEW_TEST_DATA_DIR);
    NameMappings::instance().loadFromFile(testDataDir + QStringLiteral("/name-mappings-test.json"));
    NameMappings::instance().loadFromFile(testDataDir + QStringLiteral("/vendors-test.json"));
}

void NameMappingsTest::cleanupTestCase() {
    // Reload real mappings for other tests
    NameMappings::instance().reload();
}

void NameMappingsTest::categoryNameFromGuid_known() {
    auto result = NameMappings::instance().categoryNameFromGuid(
        QStringLiteral("{4d36e96b-e325-11ce-bfc1-08002be10318}"));
    QCOMPARE(result, QStringLiteral("Keyboards"));
}

void NameMappingsTest::categoryNameFromGuid_unknown() {
    auto result = NameMappings::instance().categoryNameFromGuid(
        QStringLiteral("{00000000-0000-0000-0000-000000000000}"));
    QCOMPARE(result, QStringLiteral("Other devices"));
}

void NameMappingsTest::categoryNameFromGuid_caseInsensitive() {
    // Test uppercase
    auto upper = NameMappings::instance().categoryNameFromGuid(
        QStringLiteral("{4D36E96B-E325-11CE-BFC1-08002BE10318}"));
    // Test lowercase
    auto lower = NameMappings::instance().categoryNameFromGuid(
        QStringLiteral("{4d36e96b-e325-11ce-bfc1-08002be10318}"));
    QCOMPARE(upper, lower);
    QCOMPARE(upper, QStringLiteral("Keyboards"));
}

void NameMappingsTest::hidVendorName_known() {
    auto result = NameMappings::instance().hidVendorName(QStringLiteral("046d"));
    QCOMPARE(result, QStringLiteral("Logitech"));
}

void NameMappingsTest::hidVendorName_unknown() {
    auto result = NameMappings::instance().hidVendorName(QStringLiteral("ffff"));
    QVERIFY(result.isEmpty());
}

void NameMappingsTest::hidVendorName_caseInsensitive() {
    auto upper = NameMappings::instance().hidVendorName(QStringLiteral("046D"));
    auto lower = NameMappings::instance().hidVendorName(QStringLiteral("046d"));
    QCOMPARE(upper, lower);
    QCOMPARE(upper, QStringLiteral("Logitech"));
}

void NameMappingsTest::hidBusTypeName_knownTypes() {
    QCOMPARE(NameMappings::instance().hidBusTypeName(1), QStringLiteral("PCI"));
    QCOMPARE(NameMappings::instance().hidBusTypeName(3), QStringLiteral("USB"));
    QCOMPARE(NameMappings::instance().hidBusTypeName(5), QStringLiteral("Bluetooth"));
    QCOMPARE(NameMappings::instance().hidBusTypeName(6), QStringLiteral("Virtual"));
}

void NameMappingsTest::hidBusTypeName_unknownType() {
    auto result = NameMappings::instance().hidBusTypeName(999);
    QVERIFY(result.isEmpty());
}

void NameMappingsTest::softwareDeviceDisplayName_known() {
    auto result = NameMappings::instance().softwareDeviceDisplayName(QStringLiteral("fuse"));
    QCOMPARE(result, QStringLiteral("FUSE"));

    result = NameMappings::instance().softwareDeviceDisplayName(QStringLiteral("kvm"));
    QCOMPARE(result, QStringLiteral("KVM"));
}

void NameMappingsTest::softwareDeviceDisplayName_unknown() {
    auto result =
        NameMappings::instance().softwareDeviceDisplayName(QStringLiteral("nonexistent_device"));
    QVERIFY(result.isEmpty());
}

void NameMappingsTest::acpiDeviceDisplayName_known() {
    auto result = NameMappings::instance().acpiDeviceDisplayName(QStringLiteral("PNP0C0A"));
    QCOMPARE(result, QStringLiteral("ACPI-Compliant Control Method Battery"));
}

void NameMappingsTest::acpiDeviceDisplayName_unknown() {
    auto result = NameMappings::instance().acpiDeviceDisplayName(QStringLiteral("ZZZZ9999"));
    QVERIFY(result.isEmpty());
}

void NameMappingsTest::acpiDeviceDisplayName_caseInsensitive() {
    auto upper = NameMappings::instance().acpiDeviceDisplayName(QStringLiteral("PNP0C0A"));
    auto lower = NameMappings::instance().acpiDeviceDisplayName(QStringLiteral("pnp0c0a"));
    QCOMPARE(upper, lower);
    QCOMPARE(upper, QStringLiteral("ACPI-Compliant Control Method Battery"));
}

void NameMappingsTest::vendorSupportUrl_known() {
    auto result = NameMappings::instance().vendorSupportUrl(QStringLiteral("Intel Corporation"));
    QCOMPARE(result, QStringLiteral("https://www.intel.com/support"));

    result = NameMappings::instance().vendorSupportUrl(QStringLiteral("Logitech"));
    QCOMPARE(result, QStringLiteral("https://support.logitech.com"));
}

void NameMappingsTest::vendorSupportUrl_unknown() {
    auto result = NameMappings::instance().vendorSupportUrl(QStringLiteral("Unknown Vendor Corp"));
    QVERIFY(result.isEmpty());
}

void NameMappingsTest::clear_removesAllMappings() {
    // First verify we have data
    QVERIFY(!NameMappings::instance().hidVendorName(QStringLiteral("046d")).isEmpty());

    NameMappings::instance().clear();

    // After clear, should return empty/default values
    QVERIFY(NameMappings::instance().hidVendorName(QStringLiteral("046d")).isEmpty());
    QCOMPARE(NameMappings::instance().categoryNameFromGuid(
                 QStringLiteral("{4d36e96b-e325-11ce-bfc1-08002be10318}")),
             QStringLiteral("Other devices"));

    // Reload test data for remaining tests
    QString testDataDir = QStringLiteral(HWVIEW_TEST_DATA_DIR);
    NameMappings::instance().loadFromFile(testDataDir + QStringLiteral("/name-mappings-test.json"));
    NameMappings::instance().loadFromFile(testDataDir + QStringLiteral("/vendors-test.json"));
}

void NameMappingsTest::loadFromFile_loadsMappings() {
    NameMappings::instance().clear();

    QString testDataDir = QStringLiteral(HWVIEW_TEST_DATA_DIR);
    NameMappings::instance().loadFromFile(testDataDir + QStringLiteral("/name-mappings-test.json"));

    // Verify mappings were loaded
    QCOMPARE(NameMappings::instance().hidVendorName(QStringLiteral("046d")),
             QStringLiteral("Logitech"));
    QCOMPARE(NameMappings::instance().hidBusTypeName(3), QStringLiteral("USB"));
}

QTEST_MAIN(NameMappingsTest)
#include "namemappingstest.moc"
