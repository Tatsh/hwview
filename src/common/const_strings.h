#pragma once

#include <QRegularExpression>
#include <QString>

#ifndef DEVMGMT_CLI_ONLY
#include "const_strings_icons.h"
#endif
#include "const_strings_udev.h"
#include "namemappings.h"

/**
 * @brief Namespace containing string constants and utility functions.
 *
 * This namespace provides commonly used string constants and functions for device name mapping,
 * HID device parsing, and ACPI device naming.
 */
namespace strings {

/**
     * @brief Returns the string "1".
     * @returns Reference to the cached string.
     */
inline const QString &digit1() {
    static const QString s = QStringLiteral("1");
    return s;
}

/**
     * @brief Returns an empty string.
     * @returns Reference to the cached empty string.
     */
inline const QString &empty() {
    static const QString s;
    return s;
}

/**
     * @brief Returns a single space string.
     * @returns Reference to the cached string.
     */
inline const QString &singleSpace() {
    static const QString s = QStringLiteral(" ");
    return s;
}

/**
     * @brief Returns an underscore string.
     * @returns Reference to the cached string.
     */
inline const QString &underscore() {
    static const QString s = QStringLiteral("_");
    return s;
}

/**
     * @brief Looks up a HID vendor name by vendor ID.
     * @param vendorId The vendor ID as a hexadecimal string.
     * @returns The vendor name, or empty string if unknown.
     */
inline QString hidVendorName(const QString &vendorId) {
    return NameMappings::instance().hidVendorName(vendorId);
}

/**
     * @brief Maps a software device name to a user-friendly display name.
     * @param name The device name (may include @c /dev/ prefix).
     * @returns A user-friendly name, or the original name if no mapping exists.
     */
inline QString softwareDeviceDisplayName(const QString &name) {
    // Strip /dev/ prefix if present
    QString shortName = name;
    if (shortName.startsWith(QStringLiteral("/dev/"))) {
        shortName = shortName.mid(5);
    }

    // Handle input/event* devices
    if (shortName.startsWith(QStringLiteral("input/event"))) {
        QString num = shortName.mid(11);
        return QStringLiteral("Input event %1").arg(num);
    }
    // Handle input/mouse* devices
    if (shortName.startsWith(QStringLiteral("input/mouse"))) {
        QString num = shortName.mid(11);
        return QStringLiteral("Input mouse %1").arg(num);
    }
    // Handle i2c-N devices
    if (shortName.startsWith(QStringLiteral("i2c-"))) {
        QString num = shortName.mid(4);
        return QStringLiteral("I²C Adapter %1").arg(num);
    }
    // Handle SCSI host adapters (hostN)
    if (shortName.startsWith(QStringLiteral("host"))) {
        QString num = shortName.mid(4);
        bool ok = false;
        num.toInt(&ok);
        if (ok) {
            return QStringLiteral("SCSI Host Adapter %1").arg(num);
        }
    }
    // Handle SCSI targets (targetX:Y:Z)
    if (shortName.startsWith(QStringLiteral("target"))) {
        QString target = shortName.mid(6);
        return QStringLiteral("SCSI Target %1").arg(target);
    }

    // Parse HID device names like "PNP0C50:00 06CB:7E7E Mouse"
    static const QRegularExpression hidNameRe(
        QStringLiteral("^(?:PNP[0-9A-Fa-f]{4}|ACPI[0-9A-Fa-f]{4}):[0-9A-Fa-f]+\\s+"
                       "([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4})(?:\\s+(.+))?$"));
    auto hidMatch = hidNameRe.match(shortName);
    if (hidMatch.hasMatch()) {
        QString vendorId = hidMatch.captured(1);
        QString deviceType = hidMatch.captured(3).trimmed();
        QString vendorName = hidVendorName(vendorId);

        if (!vendorName.isEmpty()) {
            if (!deviceType.isEmpty()) {
                return QStringLiteral("%1 %2").arg(vendorName, deviceType);
            }
            return QStringLiteral("%1 HID device").arg(vendorName);
        }
        if (!deviceType.isEmpty()) {
            return deviceType;
        }
    }

    // Look up in JSON mappings
    QString displayName = NameMappings::instance().softwareDeviceDisplayName(shortName);
    if (!displayName.isEmpty()) {
        return displayName;
    }

    return shortName;
}

/**
     * @brief HID bus type constants from Linux @c input.h.
     */
namespace hidBusTypes {
constexpr int BUS_PCI = 0x01;       ///< PCI bus
constexpr int BUS_USB = 0x03;       ///< USB bus
constexpr int BUS_BLUETOOTH = 0x05; ///< Bluetooth
constexpr int BUS_VIRTUAL = 0x06;   ///< Virtual device
constexpr int BUS_I2C = 0x18;       ///< I2C HID
constexpr int BUS_HOST = 0x19;      ///< Host bus
} // namespace hidBusTypes

/**
     * @brief Parsed HID device ID information.
     */
struct HidDeviceId {
    int busType = 0;    ///< Bus type (see @c hidBusTypes)
    QString vendorId;   ///< Vendor ID as hex string
    QString productId;  ///< Product ID as hex string
    QString instance;   ///< Instance number
    bool valid = false; ///< Whether parsing succeeded
};

/**
     * @brief Parses a HID device ID from a path component.
     *
     * Parses strings like "0018:06CB:7E7E.0005" to extract bus type, vendor ID, product ID, and
     * instance number.
     *
     * @param path The path containing a HID device ID component.
     * @returns Parsed @c HidDeviceId structure.
     */
inline HidDeviceId parseHidDeviceId(const QString &path) {
    HidDeviceId result;
    static const QRegularExpression hidIdRe(
        QStringLiteral("([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4})\\.([0-9A-Fa-f]+)"));

    for (const QString &component : path.split(QLatin1Char('/'))) {
        auto match = hidIdRe.match(component);
        if (match.hasMatch()) {
            bool ok;
            result.busType = match.captured(1).toInt(&ok, 16);
            result.vendorId = match.captured(2);
            result.productId = match.captured(3);
            result.instance = match.captured(4);
            result.valid = ok;
            return result;
        }
    }
    return result;
}

/**
     * @brief Returns the human-readable name for a HID bus type.
     * @param busType The bus type constant.
     * @returns The bus type name, or empty string if unknown.
     */
inline QString hidBusTypeName(int busType) {
    return NameMappings::instance().hidBusTypeName(busType);
}

/**
     * @brief Parses an I2C bus number from a device path.
     * @param path The device path containing an I2C bus component.
     * @returns The I2C bus number, or @c -1 if not found.
     */
inline int parseI2cBusNumber(const QString &path) {
    static const QRegularExpression i2cBusRe(QStringLiteral("/i2c-(\\d+)(?:/|$)"));
    auto match = i2cBusRe.match(path);
    if (match.hasMatch()) {
        bool ok;
        int bus = match.captured(1).toInt(&ok);
        if (ok) {
            return bus;
        }
    }
    return -1;
}

/**
     * @brief Maps an ACPI/PNP device ID to a user-friendly name.
     * @param devPath The device path containing the PNP ID.
     * @param fallbackName Name to use if no mapping exists.
     * @returns A user-friendly name for the ACPI device.
     */
inline QString acpiDeviceDisplayName(const QString &devPath, const QString &fallbackName) {
    auto lastSlash = devPath.lastIndexOf(QLatin1Char('/'));
    if (lastSlash < 0) {
        return fallbackName;
    }
    QString lastComponent = devPath.mid(lastSlash + 1);

    auto colonPos = lastComponent.indexOf(QLatin1Char(':'));
    QString pnpId = colonPos > 0 ? lastComponent.left(colonPos) : lastComponent;

    // Look up in JSON mappings
    QString displayName = NameMappings::instance().acpiDeviceDisplayName(pnpId);
    if (!displayName.isEmpty()) {
        return displayName;
    }

    // Fallback: capitalize simple names like "battery" -> "Battery"
    if (!fallbackName.isEmpty()) {
        QString result = fallbackName;
        result[0] = result[0].toUpper();
        return result;
    }
    return fallbackName;
}

} // namespace strings
