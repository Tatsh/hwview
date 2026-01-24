#pragma once

#include <QHash>
#include <QString>

/**
 * @brief Manages device name mappings loaded from JSON files.
 *
 * This singleton class loads name mappings from JSON files in standard locations. User-specific
 * mappings in @c ~/.local/share/devmgmt (or equivalent) override system mappings in
 * @c /usr/share/devmgmt (or equivalent).
 *
 * The JSON file format is:
 * @code
 * {
 *   "guid-to-category": { "{GUID}": "Category Name", ... },
 *   "hid-vendor": { "046d": "Logitech", ... },
 *   "hid-bus-type": { "3": "USB", ... },
 *   "software-device": { "fuse": "FUSE", ... },
 *   "acpi-device": { "PNP0C0A": "Microsoft ACPI-Compliant Control Method Battery", ... }
 * }
 * @endcode
 */
class NameMappings {
public:
    /**
     * @brief Returns the singleton instance.
     * @returns Reference to the global @c NameMappings instance.
     */
    static NameMappings &instance();

    /**
     * @brief Returns the category name for a GUID string.
     * @param guidString The GUID in string format
     *                   (e.g., "{4d36e96b-e325-11ce-bfc1-08002be10318}").
     * @returns The category name, or "Other devices" if not found.
     */
    QString categoryNameFromGuid(const QString &guidString) const;

    /**
     * @brief Returns the vendor name for a HID vendor ID.
     * @param vendorId The vendor ID as a hexadecimal string (e.g., "046d").
     * @returns The vendor name, or empty string if not found.
     */
    QString hidVendorName(const QString &vendorId) const;

    /**
     * @brief Returns the human-readable name for a HID bus type.
     * @param busType The bus type constant (e.g., 3 for USB).
     * @returns The bus type name, or empty string if not found.
     */
    QString hidBusTypeName(int busType) const;

    /**
     * @brief Returns the nice name for a software device.
     * @param deviceName The device name (without @c /dev/ prefix).
     * @returns The nice name, or empty string if not found.
     */
    QString softwareDeviceNiceName(const QString &deviceName) const;

    /**
     * @brief Returns the nice name for an ACPI/PNP device ID.
     * @param pnpId The PNP ID (e.g., "PNP0C0A").
     * @returns The nice name, or empty string if not found.
     */
    QString acpiDeviceNiceName(const QString &pnpId) const;

    /**
     * @brief Reloads all mappings from JSON files.
     *
     * Clears existing mappings and reloads from system and user locations.
     */
    void reload();

private:
    NameMappings();
    ~NameMappings() = default;
    NameMappings(const NameMappings &) = delete;
    NameMappings &operator=(const NameMappings &) = delete;

    void loadFromFile(const QString &filePath);

    QHash<QString, QString> guidToCategory_;
    QHash<QString, QString> hidVendor_;
    QHash<int, QString> hidBusType_;
    QHash<QString, QString> softwareDevice_;
    QHash<QString, QString> acpiDevice_;
};
