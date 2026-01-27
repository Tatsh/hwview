// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "deviceinfo.h"

/**
 * @brief Property mapping for Details tab - maps display names to property keys.
 */
struct PropertyMapping {
    QString displayName; ///< User-visible property name.
    QString propertyKey; ///< Internal property key for lookup.
    bool isMultiValue;   ///< Whether the property can have multiple values.
};

/**
 * @brief Special property keys for the Details tab.
 *
 * These keys are handled specially and are not direct device property lookups.
 */
namespace PropertyKeys {
inline const QString deviceDescription() {
    return QStringLiteral("DEVICE_DESCRIPTION");
}
inline const QString syspath() {
    return QStringLiteral("SYSPATH");
}
inline const QString parentSyspath() {
    return QStringLiteral("PARENT_SYSPATH");
}
inline const QString children() {
    return QStringLiteral("CHILDREN");
}
inline const QString compatibleIds() {
    return QStringLiteral("COMPATIBLE_IDS");
}
inline const QString mountPoint() {
    return QStringLiteral("MOUNT_POINT");
}
} // namespace PropertyKeys

/**
 * @brief Get platform-specific property mappings for the Details tab.
 * @returns List of property mappings appropriate for the current platform.
 */
QList<PropertyMapping> getDevicePropertyMappings();

/**
 * @brief Convert a device property value to Windows-style hardware IDs.
 *
 * On Linux/macOS, this parses modalias values and returns equivalent Windows hardware IDs.
 * On Windows, this returns the input value as-is (already in Windows format).
 *
 * @param propertyKey The property key (e.g., "MODALIAS" on Linux).
 * @param value The property value to convert.
 * @returns List of Windows-style hardware IDs, or empty list if not applicable.
 */
QStringList convertToHardwareIds(const QString &propertyKey, const QString &value);

/**
 * @brief Basic driver information for the Driver tab.
 *
 * This is different from the detailed DriverInfo in driverinfo.h which contains
 * comprehensive module information for the driver details dialog.
 */
struct BasicDriverInfo {
    QString provider;            ///< Driver provider or vendor name.
    QString version;             ///< Driver version string.
    QString signer;              ///< Digital signature signer.
    QString date;                ///< Driver date or build date.
    bool hasDriverFiles = false; ///< Whether driver files were found.
};

/**
 * @brief Get basic driver information for a device driver.
 *
 * This function may block while querying system information (modinfo, kextstat, registry).
 * Call from a background thread.
 *
 * @param driver Driver name/identifier.
 * @returns Basic driver information including provider, version, signer, and date.
 */
BasicDriverInfo getBasicDriverInfo(const QString &driver);

/**
 * @brief Information about a built-in driver for display purposes.
 */
struct BuiltinDriverInfo {
    QString provider;       ///< Driver provider name.
    QString version;        ///< Kernel version string.
    QString copyright;      ///< Copyright notice.
    QString signer;         ///< Signature signer.
    QString builtinMessage; ///< Localized message like "(Built-in kernel module)".
};

/**
 * @brief Get display information for a built-in driver.
 * @returns Platform-specific built-in driver display info.
 */
BuiltinDriverInfo getBuiltinDriverInfo();

/**
 * @brief Driver file details for the driver details dialog.
 */
struct DriverFileDetails {
    QString provider;  ///< Driver provider or vendor.
    QString version;   ///< Driver version string.
    QString copyright; ///< Copyright notice.
    QString signer;    ///< Digital signature signer.
};

/**
 * @brief Get display details for a driver file.
 *
 * Extracts provider, version, copyright, and signer information from a driver file.
 * Uses platform-specific methods (modinfo on Linux, kextstat on macOS, registry on Windows).
 *
 * @param driverPath Path to the driver file.
 * @param driverName Name of the driver module.
 * @returns Driver file details.
 */
DriverFileDetails getDriverFileDetails(const QString &driverPath, const QString &driverName);

/**
 * @brief Format a driver file path for display.
 *
 * On Windows, converts to native path separators. On other platforms, returns as-is.
 *
 * @param path The driver file path.
 * @returns Formatted path for display.
 */
QString formatDriverPath(const QString &path);

/**
 * @brief Get display name for a device category.
 * @param category The device category enum value.
 * @param fallback Fallback string to use for Unknown category (typically subsystem name).
 * @returns Localized display name for the category.
 */
QString getCategoryDisplayName(DeviceCategory category, const QString &fallback);

/**
 * @brief Get manufacturer name for a device.
 *
 * Tries multiple sources to determine manufacturer: vendor database, USB vendor,
 * URL-encoded vendor, device path parsing, etc.
 *
 * @param info The device info to get manufacturer for.
 * @returns Manufacturer name or empty string if not found.
 */
QString getDeviceManufacturer(const DeviceInfo &info);

/**
 * @brief Check if a syspath represents the Computer root entry.
 * @param syspath The device system path.
 * @returns true if this is the Computer entry.
 */
bool isComputerEntry(const QString &syspath);

/**
 * @brief Get the display name for a device.
 *
 * Applies platform-specific name mappings for software devices, batteries,
 * storage volumes, etc. to produce a user-friendly display name.
 *
 * @param info The device info.
 * @returns User-friendly display name for the device.
 */
QString getDeviceDisplayName(const DeviceInfo &info);

/**
 * @brief Check if a device has driver information to display.
 *
 * On Linux, this returns true for devices with drivers or "Linux Foundation" devices
 * (misc, input, partitions). On other platforms, this typically returns true if
 * the device has an associated driver.
 *
 * @param info The device info.
 * @returns true if the Driver tab should show driver information.
 */
bool hasDriverInfo(const DeviceInfo &info);

/**
 * @brief Get the display name for the Computer entry.
 *
 * Returns a localized name based on system type (e.g., "ACPI x64-based PC",
 * "Apple Silicon Mac", "x64-based PC").
 *
 * @returns Localized computer display name.
 */
QString getComputerDisplayName();

/**
 * @brief Get the local hostname.
 *
 * Returns the hostname of the local machine without requiring Qt6::Network.
 *
 * @returns Local hostname, or "unknown" if it cannot be determined.
 */
QString getHostname();

/**
 * @brief Get the syspath for the Computer entry.
 *
 * Returns the platform-specific path used to identify the root computer node
 * (e.g., "/sys/devices/virtual/dmi/id" on Linux, "IOService:/" on macOS).
 *
 * @returns Platform-specific computer syspath.
 */
QString getComputerSyspath();

/**
 * @brief Open the platform's printers/devices settings.
 *
 * Opens the appropriate settings panel for managing printers and devices
 * on the current platform (Control Panel on Windows, System Preferences on macOS,
 * System Settings on Linux).
 */
void openPrintersSettings();

/**
 * @brief Get the kernel/OS version string.
 * @returns Version string (e.g., "6.12.0-gentoo" on Linux, "24.1.0" on macOS, "10.0.19045" on
 * Windows).
 */
QString getKernelVersion();

/**
 * @brief Get the kernel build date formatted according to user locale.
 * @returns Localized date string or version info if date unavailable.
 */
QString getKernelBuildDate();

/**
 * @brief Translate a device path to a human-readable location string.
 * @param devpath The device path (sysfs path on Linux, IORegistry path on macOS, instance ID on
 * Windows).
 * @returns Human-readable location like "PCI bus 0, device 31, function 3" or "On USB bus 1, port
 * 2".
 */
QString translateDevicePath(const QString &devpath);

/**
 * @brief Get the mount point for a device node.
 * @param devnode The device node path (e.g., "/dev/sda1").
 * @returns Mount point path or empty string if not mounted.
 */
QString getMountPoint(const QString &devnode);

/**
 * @brief Look up USB vendor name from vendor ID.
 * @param vendorId 4-character hex vendor ID (e.g., "046d").
 * @returns Vendor name or empty string if not found.
 */
QString lookupUsbVendor(const QString &vendorId);

/**
 * @brief Query parameters for device event search.
 */
struct DeviceEventQuery {
    QString syspath;    ///< Device system path.
    QString devnode;    ///< Device node path (e.g., /dev/sda).
    QString deviceName; ///< Human-readable device name.
    QString vendorId;   ///< Vendor ID for matching.
    QString modelId;    ///< Model/product ID for matching.
};

/**
 * @brief Build a device event query from device info using platform-appropriate properties.
 * @param info The device to build a query for.
 * @returns Query populated with relevant device identifiers.
 */
DeviceEventQuery buildEventQuery(const DeviceInfo &info);

/**
 * @brief Query system logs for events related to a device.
 * @param query Device identifiers to search for.
 * @returns List of raw log lines (up to 50 events).
 */
QStringList queryDeviceEvents(const DeviceEventQuery &query);

/**
 * @brief Parsed event with separated timestamp and message.
 */
struct ParsedEvent {
    QString timestamp; ///< Event timestamp.
    QString message;   ///< Event message content.
};

/**
 * @brief Parse a platform-specific log line into timestamp and message.
 * @param line Raw log line from queryDeviceEvents().
 * @returns ParsedEvent with separated fields.
 */
ParsedEvent parseEventLine(const QString &line);

/**
 * @brief Hardware resource information.
 */
struct ResourceInfo {
    QString type;     ///< Resource type (e.g., "IRQ", "Memory", "I/O Port").
    QString setting;  ///< Resource value or range.
    QString iconName; ///< Icon name for display.
};

/**
 * @brief Get hardware resources (IRQ, memory, I/O) for a device.
 * @param syspath Device path (sysfs on Linux, IORegistry on macOS, instance ID on Windows).
 * @param driver Driver name (used on Linux for DMA lookup).
 * @returns List of resources.
 */
QList<ResourceInfo> getDeviceResources(const QString &syspath, const QString &driver);

/**
 * @brief DMA channel information.
 */
struct DmaChannelInfo {
    QString channel; ///< DMA channel number.
    QString name;    ///< Device using this channel.
};

/**
 * @brief Get all system DMA channels.
 * @returns List of DMA channels.
 */
QList<DmaChannelInfo> getSystemDmaChannels();

/**
 * @brief I/O port range with hierarchy support.
 */
struct IoPortInfo {
    QString rangeStart;  ///< Starting port address (hex).
    QString rangeEnd;    ///< Ending port address (hex).
    QString name;        ///< Device or region name.
    int indentLevel = 0; ///< Hierarchy level for nested regions.
};

/**
 * @brief Get all system I/O port ranges.
 * @returns List of I/O port ranges with indentation for hierarchy.
 */
QList<IoPortInfo> getSystemIoPorts();

/**
 * @brief IRQ information.
 */
struct IrqInfo {
    QString irqNumber;  ///< IRQ number.
    QString irqType;    ///< IRQ type (e.g., "IO-APIC", "PCI-MSI").
    QString deviceName; ///< Device using this IRQ.
};

/**
 * @brief Get all system IRQs.
 * @returns List of IRQ information.
 */
QList<IrqInfo> getSystemIrqs();

/**
 * @brief Memory range with hierarchy support.
 */
struct MemoryRangeInfo {
    QString rangeStart;  ///< Starting memory address (hex).
    QString rangeEnd;    ///< Ending memory address (hex).
    QString name;        ///< Device or region name.
    int indentLevel = 0; ///< Hierarchy level for nested regions.
};

/**
 * @brief Get all system memory ranges.
 * @returns List of memory ranges with indentation for hierarchy.
 */
QList<MemoryRangeInfo> getSystemMemoryRanges();

/**
 * @brief System uname information for export.
 */
struct UnameInfo {
    QString sysname;    ///< Operating system name (e.g., "Linux").
    QString release;    ///< Kernel release version.
    QString version;    ///< Kernel version and build info.
    QString machine;    ///< Hardware architecture (e.g., "x86_64").
    bool valid = false; ///< Whether uname call succeeded.
};

/**
 * @brief Get uname system information.
 * @returns Uname info structure.
 */
UnameInfo getUnameInfo();

/**
 * @brief Get OS distribution information (Linux-specific).
 * @returns Map of key-value pairs from /etc/os-release, or empty map on non-Linux.
 */
QHash<QString, QString> getDistributionInfo();

/**
 * @brief Get additional device properties for export.
 *
 * Returns platform-specific properties that are useful for device identification
 * and export purposes.
 *
 * @param info The device to get properties for.
 * @returns Map of property names to values (empty values filtered out).
 */
QHash<QString, QString> getExportDeviceProperties(const DeviceInfo &info);

/**
 * @brief Device resource for export.
 */
struct ExportResourceInfo {
    QString type;         ///< Resource type identifier.
    QString displayValue; ///< Human-readable resource value.
    QString start;        ///< Resource start address or number.
    QString end;          ///< Resource end address (for ranges).
    QString flags;        ///< Resource flags or attributes.
    int value = 0;        ///< Numeric value (e.g., IRQ number).
};

/**
 * @brief Get device resources for export.
 * @param syspath Device system path.
 * @returns List of resource information for export.
 */
QList<ExportResourceInfo> getExportDeviceResources(const QString &syspath);

/**
 * @brief Driver information for export.
 */
struct ExportDriverInfo {
    bool hasDriver = false;   ///< Whether the device has a driver.
    QString name;             ///< Driver/module name.
    QString filename;         ///< Path to driver file.
    QString author;           ///< Driver author.
    QString version;          ///< Driver version string.
    QString license;          ///< License type.
    QString description;      ///< Driver description.
    QString signer;           ///< Digital signature signer.
    QString srcversion;       ///< Source version hash.
    QString vermagic;         ///< Kernel version magic string.
    QString date;             ///< Driver date.
    QString bundleIdentifier; ///< macOS bundle identifier.
    QString provider;         ///< Driver provider/vendor.
    bool isOutOfTree = false; ///< Whether driver is out-of-tree (external).
    bool isBuiltin = false;   ///< Whether driver is built into kernel.
};

/**
 * @brief Get driver information for export.
 * @param info The device to get driver information for.
 * @returns Driver information for export.
 */
ExportDriverInfo getExportDriverInfo(const DeviceInfo &info);

/**
 * @brief Get raw system resource files content for export.
 *
 * On Linux, reads /proc/dma, /proc/ioports, /proc/interrupts, /proc/iomem.
 * On other platforms, returns empty values.
 *
 * @returns Map of resource type to raw file content.
 */
QHash<QString, QString> getSystemResourcesRaw();

/**
 * @brief Enumerate all devices on the system.
 *
 * This is the primary device enumeration function that uses the platform-specific
 * backend (udev on Linux, IOKit on macOS, SetupAPI on Windows) to enumerate devices.
 *
 * @returns List of all discovered devices.
 */
QList<DeviceInfo> enumerateAllDevices();

/**
 * @brief Create a device monitor for tracking device changes.
 *
 * The returned QObject will emit a "deviceChanged()" signal when devices are
 * added or removed. The caller is responsible for deleting the monitor.
 *
 * @param parent Parent QObject for memory management.
 * @returns A platform-specific device monitor, or nullptr if monitoring is not supported.
 */
QObject *createDeviceMonitor(QObject *parent);
