#pragma once

#include <QHash>
#include <QString>

/**
 * @brief Manages device name mappings loaded from JSON files.
 *
 * This singleton class loads name mappings from JSON files from multiple locations. Mappings are
 * loaded in the following order, with later files overriding earlier ones at the key level:
 *
 * 1. Adjacent @c data/ directory (relative to executable, for development/portable use)
 * 2. System locations (@c /usr/share/devmgmt, etc.)
 * 3. User location (@c ~/.local/share/devmgmt, etc.)
 *
 * Within each location, files are loaded in this order:
 * - Default file (@c name-mappings.json or @c name-mappings.en-US.json)
 * - Locale-specific file (@c name-mappings.{locale}.json)
 *
 * Locale-specific files only need to define the keys they want to override. Keys not present in
 * locale-specific files retain their values from the default file.
 *
 * System locations:
 * - Linux: @c /usr/share/devmgmt, @c /usr/local/share/devmgmt
 * - Windows: @c C:/ProgramData/devmgmt
 * - macOS: @c /Library/Application Support/devmgmt
 *
 * User locations:
 * - Linux: @c ~/.local/share/devmgmt
 * - Windows: @c C:/Users/{USER}/AppData/Local/devmgmt
 * - macOS: @c ~/Library/Application Support/devmgmt
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
     * @brief Returns the display name for a software device.
     * @param deviceName The device name (without @c /dev/ prefix).
     * @returns The display name, or empty string if not found.
     */
    QString softwareDeviceDisplayName(const QString &deviceName) const;

    /**
     * @brief Returns the display name for an ACPI/PNP device ID.
     * @param pnpId The PNP ID (e.g., "PNP0C0A").
     * @returns The display name, or empty string if not found.
     */
    QString acpiDeviceDisplayName(const QString &pnpId) const;

    /**
     * @brief Reloads all mappings from JSON files.
     *
     * Clears existing mappings and reloads from system and user locations, respecting locale.
     */
    void reload();

private:
    NameMappings();
    ~NameMappings() = default;
    NameMappings(const NameMappings &) = delete;
    NameMappings &operator=(const NameMappings &) = delete;

    void loadFromFile(const QString &filePath);
    void loadFromDirectory(const QString &dirPath, const QString &locale);
    QString systemLocale() const;

    QHash<QString, QString> guidToCategory_;
    QHash<QString, QString> hidVendor_;
    QHash<int, QString> hidBusType_;
    QHash<QString, QString> softwareDevice_;
    QHash<QString, QString> acpiDevice_;

    static constexpr const char *kDefaultLocale = "en-US";
};
