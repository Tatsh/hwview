#include "namemappings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

NameMappings &NameMappings::instance() {
    static NameMappings mappings;
    return mappings;
}

NameMappings::NameMappings() {
    reload();
}

void NameMappings::reload() {
    guidToCategory_.clear();
    hidVendor_.clear();
    hidBusType_.clear();
    softwareDevice_.clear();
    acpiDevice_.clear();

    // Get standard data locations
    // On Linux: /usr/share, /usr/local/share, etc.
    // On Windows: C:/ProgramData, etc.
    // On macOS: /Library/Application Support, etc.
    QStringList systemPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    // Load from system locations first (in reverse order so higher priority paths override)
    for (int i = systemPaths.size() - 1; i >= 0; --i) {
        QString filePath = systemPaths.at(i) + QStringLiteral("/devmgmt/name-mappings.json");
        loadFromFile(filePath);
    }

    // Load from user-specific location last (highest priority, overrides system)
    // On Linux: ~/.local/share/devmgmt
    // On Windows: C:/Users/<USER>/AppData/Local/devmgmt
    // On macOS: ~/Library/Application Support/devmgmt
    QString userPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                       QStringLiteral("/devmgmt/name-mappings.json");
    loadFromFile(userPath);
}

void NameMappings::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    QJsonObject root = doc.object();

    // Load GUID to category mappings
    if (root.contains(QStringLiteral("guid-to-category"))) {
        QJsonObject mappings = root.value(QStringLiteral("guid-to-category")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                // Normalize GUID to lowercase for consistent lookup
                QString guid = it.key().toLower();
                guidToCategory_.insert(guid, it.value().toString());
            }
        }
    }

    // Load HID vendor mappings
    if (root.contains(QStringLiteral("hid-vendor"))) {
        QJsonObject mappings = root.value(QStringLiteral("hid-vendor")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                // Normalize to lowercase for consistent lookup
                QString vendorId = it.key().toLower();
                hidVendor_.insert(vendorId, it.value().toString());
            }
        }
    }

    // Load HID bus type mappings
    if (root.contains(QStringLiteral("hid-bus-type"))) {
        QJsonObject mappings = root.value(QStringLiteral("hid-bus-type")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                bool ok;
                int busType = it.key().toInt(&ok);
                if (ok) {
                    hidBusType_.insert(busType, it.value().toString());
                }
            }
        }
    }

    // Load software device mappings
    if (root.contains(QStringLiteral("software-device"))) {
        QJsonObject mappings = root.value(QStringLiteral("software-device")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                softwareDevice_.insert(it.key(), it.value().toString());
            }
        }
    }

    // Load ACPI device mappings
    if (root.contains(QStringLiteral("acpi-device"))) {
        QJsonObject mappings = root.value(QStringLiteral("acpi-device")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                // Store with uppercase key for consistent lookup
                QString pnpId = it.key().toUpper();
                acpiDevice_.insert(pnpId, it.value().toString());
            }
        }
    }
}

QString NameMappings::categoryNameFromGuid(const QString &guidString) const {
    // Normalize to lowercase for lookup
    QString normalizedGuid = guidString.toLower();
    return guidToCategory_.value(normalizedGuid, QStringLiteral("Other devices"));
}

QString NameMappings::hidVendorName(const QString &vendorId) const {
    // Normalize to lowercase for lookup
    QString normalizedId = vendorId.toLower();
    return hidVendor_.value(normalizedId);
}

QString NameMappings::hidBusTypeName(int busType) const {
    return hidBusType_.value(busType);
}

QString NameMappings::softwareDeviceNiceName(const QString &deviceName) const {
    return softwareDevice_.value(deviceName);
}

QString NameMappings::acpiDeviceNiceName(const QString &pnpId) const {
    // Normalize to uppercase for lookup
    QString normalizedId = pnpId.toUpper();
    return acpiDevice_.value(normalizedId);
}
