// SPDX-License-Identifier: MIT
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QLocale>
#include <QtCore/QStandardPaths>

#include "namemappings.h"

NameMappings &NameMappings::instance() {
    static NameMappings mappings;
    return mappings;
}

NameMappings::NameMappings() {
    reload();
}

void NameMappings::clear() {
    guidToCategory_.clear();
    hidVendor_.clear();
    hidBusType_.clear();
    softwareDevice_.clear();
    acpiDevice_.clear();
    vendorUrls_.clear();
}

// LCOV_EXCL_START - Platform-specific locale detection
QString NameMappings::systemLocale() const {
    // Get locale in format like "en-US", "de-DE", "ja-JP"
    auto localeName = QLocale::system().name(); // Returns "en_US" format
    localeName.replace(QLatin1Char('_'), QLatin1Char('-'));
    return localeName;
}
// LCOV_EXCL_STOP

void NameMappings::reload() {
    clear();

    // LCOV_EXCL_START - Platform-specific path resolution
    auto locale = systemLocale();

    // Load from data/ directory adjacent to the executable first (lowest priority)
    // This allows running the application without installation during development
    auto appDir = QCoreApplication::applicationDirPath();
    loadFromDirectory(appDir + QStringLiteral("/data"), locale);
    // Also check parent directories (for when binary is in build/src/ or build/src/viewer/)
    loadFromDirectory(appDir + QStringLiteral("/../data"), locale);
    loadFromDirectory(appDir + QStringLiteral("/../../data"), locale);
    loadFromDirectory(appDir + QStringLiteral("/../../../data"), locale);

    // Get standard data locations
    // On Linux: /usr/share, /usr/local/share, etc.
    // On Windows: C:/ProgramData, etc.
    // On macOS: /Library/Application Support, etc.
    auto systemPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    // Load from system locations (in reverse order so higher priority paths override)
    for (auto i = systemPaths.size() - 1; i >= 0; --i) {
        auto dirPath = systemPaths.at(i) + QStringLiteral("/hwview");
        loadFromDirectory(dirPath, locale);
    }

    // Load from user-specific location (highest priority, overrides system)
    // On Linux: ~/.local/share/hwview
    // On Windows: C:/Users/<USER>/AppData/Local/hwview
    // On macOS: ~/Library/Application Support/hwview
    auto userPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                    QStringLiteral("/hwview");
    loadFromDirectory(userPath, locale);

#ifndef Q_OS_MACOS
    // On non-macOS, check exe_path/../share/hwview/ for FHS-style installations
    // This allows the application to find data when installed to any prefix
    loadFromDirectory(appDir + QStringLiteral("/../share/hwview"), locale);
#else
    // On macOS, check the bundle's Resources directory (after user locations)
    // applicationDirPath() returns AppName.app/Contents/MacOS, Resources is ../Resources
    loadFromDirectory(appDir + QStringLiteral("/../Resources"), locale);
#endif
    // LCOV_EXCL_STOP
}

void NameMappings::loadFromDirectory(const QString &dirPath, const QString &locale) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }

    // Load default file first (en-US)
    // Try name-mappings.en-US.json first, fall back to name-mappings.json
    auto defaultLocalePath = dirPath + QStringLiteral("/name-mappings.") +
                             QLatin1String(kDefaultLocale) + QStringLiteral(".json");
    auto defaultPath = dirPath + QStringLiteral("/name-mappings.json");

    if (QFile::exists(defaultLocalePath)) {
        loadFromFile(defaultLocalePath); // LCOV_EXCL_LINE
    } else if (QFile::exists(defaultPath)) {
        loadFromFile(defaultPath);
    }

    // Load locale-specific file if different from default
    if (locale != QLatin1String(kDefaultLocale)) {
        auto localePath =
            dirPath + QStringLiteral("/name-mappings.") + locale + QStringLiteral(".json");
        if (QFile::exists(localePath)) {
            loadFromFile(localePath); // LCOV_EXCL_LINE
        }
    }

    // Load vendors.json (not locale-specific)
    auto vendorsPath = dirPath + QStringLiteral("/vendors.json");
    if (QFile::exists(vendorsPath)) {
        QFile file(vendorsPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonParseError error;
            auto doc = QJsonDocument::fromJson(file.readAll(), &error);
            file.close();

            if (error.error == QJsonParseError::NoError && doc.isObject()) {
                auto root = doc.object();
                for (auto it = root.begin(); it != root.end(); ++it) {
                    if (it.value().isString()) {
                        vendorUrls_.insert(it.key(), it.value().toString());
                    }
                }
            }
        }
    }
}

void NameMappings::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    auto root = doc.object();

    // Load GUID to category mappings
    if (root.contains(QStringLiteral("guid-to-category"))) {
        auto mappings = root.value(QStringLiteral("guid-to-category")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                // Normalise GUID to lowercase for consistent lookup
                auto guid = it.key().toLower();
                guidToCategory_.insert(guid, it.value().toString());
            }
        }
    }

    // Load HID vendor mappings
    if (root.contains(QStringLiteral("hid-vendor"))) {
        auto mappings = root.value(QStringLiteral("hid-vendor")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                // Normalise to lowercase for consistent lookup
                auto vendorId = it.key().toLower();
                hidVendor_.insert(vendorId, it.value().toString());
            }
        }
    }

    // Load HID bus type mappings
    if (root.contains(QStringLiteral("hid-bus-type"))) {
        auto mappings = root.value(QStringLiteral("hid-bus-type")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                bool ok;
                auto busType = it.key().toInt(&ok);
                if (ok) {
                    hidBusType_.insert(busType, it.value().toString());
                }
            }
        }
    }

    // Load software device mappings
    if (root.contains(QStringLiteral("software-device"))) {
        auto mappings = root.value(QStringLiteral("software-device")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                softwareDevice_.insert(it.key(), it.value().toString());
            }
        }
    }

    // Load ACPI device mappings
    if (root.contains(QStringLiteral("acpi-device"))) {
        auto mappings = root.value(QStringLiteral("acpi-device")).toObject();
        for (auto it = mappings.begin(); it != mappings.end(); ++it) {
            if (it.value().isString()) {
                // Store with uppercase key for consistent lookup
                auto pnpId = it.key().toUpper();
                acpiDevice_.insert(pnpId, it.value().toString());
            }
        }
    }

    // If the file is a simple vendor URL mapping (like vendors.json),
    // check if all top-level keys are strings (URL values) and load them
    bool isVendorFile = true;
    for (auto it = root.begin(); it != root.end(); ++it) {
        if (!it.value().isString()) {
            isVendorFile = false;
            break;
        }
    }
    if (isVendorFile && !root.isEmpty()) {
        for (auto it = root.begin(); it != root.end(); ++it) {
            vendorUrls_.insert(it.key(), it.value().toString());
        }
    }
}

QString NameMappings::categoryNameFromGuid(const QString &guidString) const {
    // Normalise to lowercase for lookup
    auto normalizedGuid = guidString.toLower();
    return guidToCategory_.value(normalizedGuid, QStringLiteral("Other devices"));
}

QString NameMappings::hidVendorName(const QString &vendorId) const {
    // Normalise to lowercase for lookup
    auto normalizedId = vendorId.toLower();
    return hidVendor_.value(normalizedId);
}

QString NameMappings::hidBusTypeName(int busType) const {
    return hidBusType_.value(busType);
}

QString NameMappings::softwareDeviceDisplayName(const QString &deviceName) const {
    return softwareDevice_.value(deviceName);
}

QString NameMappings::acpiDeviceDisplayName(const QString &pnpId) const {
    // Normalise to uppercase for lookup
    auto normalizedId = pnpId.toUpper();
    return acpiDevice_.value(normalizedId);
}

QString NameMappings::vendorSupportUrl(const QString &vendorName) const {
    return vendorUrls_.value(vendorName);
}
