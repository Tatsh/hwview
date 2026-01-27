// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QString>

#include <memory>

class DeviceInfoPrivate;

/**
 * @brief Pre-computed device category for fast classification.
 *
 * Devices are categorised at construction time to avoid repeated classification logic during
 * display.
 */
enum class DeviceCategory {
    Unknown,                       ///< Unknown or unclassified device.
    AudioInputsAndOutputs,         ///< Audio input/output devices.
    Batteries,                     ///< Battery and power supply devices.
    Computer,                      ///< The computer itself.
    DiskDrives,                    ///< Hard disk and solid state drives.
    DisplayAdapters,               ///< Graphics cards and display adapters.
    DvdCdromDrives,                ///< Optical disc drives.
    HumanInterfaceDevices,         ///< Generic HID devices.
    Keyboards,                     ///< Keyboard devices.
    MiceAndOtherPointingDevices,   ///< Mice, touchpads, and pointing devices.
    NetworkAdapters,               ///< Network interface cards.
    SoftwareDevices,               ///< Virtual and software-based devices.
    SoundVideoAndGameControllers,  ///< Multimedia controllers.
    StorageControllers,            ///< Storage host controllers.
    StorageVolumes,                ///< Disk partitions and volumes.
    SystemDevices,                 ///< System and bridge devices.
    UniversalSerialBusControllers, ///< USB host controllers.
};

/**
 * @brief Encapsulates information about a hardware device.
 *
 * This class provides a cross-platform abstraction for device information. On Linux, it wraps
 * @c udev device data. On macOS, it wraps @c IOKit data. On Windows, it wraps @c SetupAPI data.
 *
 * @c DeviceInfo objects are copyable and movable. On Linux, copying uses @c udev reference
 * counting for efficient memory management.
 */
class DeviceInfo {
    Q_DECLARE_PRIVATE(DeviceInfo)
public:
    /**
     * @brief Constructs a @c DeviceInfo from a private implementation.
     * @param d The private implementation (takes ownership).
     */
    explicit DeviceInfo(DeviceInfoPrivate *d);

    /**
     * @brief Constructs a @c DeviceInfo from exported JSON data.
     * @param json The JSON object containing device data from an export file.
     *
     * This constructor is used when loading devices from a .dmexport file for viewing.
     */
    explicit DeviceInfo(const QJsonObject &json);

    ~DeviceInfo();

    /**
     * @brief Copy constructor.
     * @param other The @c DeviceInfo to copy from.
     */
    DeviceInfo(const DeviceInfo &other);

    /**
     * @brief Copy assignment operator.
     * @param other The @c DeviceInfo to copy from.
     * @returns Reference to this object.
     */
    DeviceInfo &operator=(const DeviceInfo &other);

    /**
     * @brief Move constructor.
     * @param other The @c DeviceInfo to move from.
     */
    DeviceInfo(DeviceInfo &&other) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other The @c DeviceInfo to move from.
     * @returns Reference to this object.
     */
    DeviceInfo &operator=(DeviceInfo &&other) noexcept;

    /**
     * @brief Returns the driver name for this device.
     * @returns The driver name, or empty string if no driver.
     */
    const QString &driver() const;

    /**
     * @brief Returns the display name of the device.
     * @returns The device name.
     */
    const QString &name() const;

    /**
     * @brief Returns a property value by key.
     * @param key The property key name.
     * @returns The property value, or empty string if not found.
     */
    QString propertyValue(const char *key) const;

    /**
     * @brief Returns the system path of the device.
     * @returns The syspath (e.g., "/sys/devices/pci0000:00/...").
     */
    const QString &syspath() const;

    /**
     * @brief Returns the parent device's system path.
     * @returns The parent syspath, or empty string if no parent.
     */
    const QString &parentSyspath() const;

    /**
     * @brief Returns the subsystem this device belongs to.
     * @returns The subsystem name (e.g., "pci", "usb", "block").
     */
    const QString &subsystem() const;

    /**
     * @brief Returns the device node path.
     * @returns The devnode (e.g., "/dev/sda"), or empty if none.
     */
    const QString &devnode() const;

    /**
     * @brief Dumps device information to standard output for debugging.
     */
    void dump();

    /**
     * @brief Returns the cached @c DEVPATH property value.
     * @returns The device path.
     */
    const QString &devPath() const;

    /**
     * @brief Returns the cached PCI class from database.
     * @returns The PCI class name.
     */
    const QString &pciClass() const;

    /**
     * @brief Returns the cached PCI subclass from database.
     * @returns The PCI subclass name.
     */
    const QString &pciSubclass() const;

    /**
     * @brief Returns the cached PCI interface from database.
     * @returns The PCI interface name.
     */
    const QString &pciInterface() const;

    /**
     * @brief Returns the cached @c ID_CDROM property.
     * @returns "1" if this is a CD-ROM device, empty otherwise.
     */
    const QString &idCdrom() const;

    /**
     * @brief Returns the cached @c DEVTYPE property.
     * @returns The device type (e.g., "disk", "partition").
     */
    const QString &devType() const;

    /**
     * @brief Returns the cached @c ID_INPUT_KEYBOARD property.
     * @returns "1" if this is a keyboard, empty otherwise.
     */
    const QString &idInputKeyboard() const;

    /**
     * @brief Returns the cached @c ID_INPUT_MOUSE property.
     * @returns "1" if this is a mouse, empty otherwise.
     */
    const QString &idInputMouse() const;

    /**
     * @brief Returns the cached @c ID_TYPE property.
     * @returns The ID type (e.g., "audio", "disk").
     */
    const QString &idType() const;

    /**
     * @brief Returns the cached model name from hardware database.
     * @returns The model name from @c udev database.
     */
    const QString &idModelFromDatabase() const;

    /**
     * @brief Returns all device properties as a JSON object.
     *
     * For live devices, this returns an empty object. For imported devices,
     * this returns the properties stored in the export file.
     *
     * @returns The properties JSON object.
     */
    const QJsonObject &properties() const;

    /**
     * @brief Returns driver information as a JSON object.
     *
     * For live devices, this returns an empty object. For imported devices,
     * this returns the driver info stored in the export file.
     *
     * @returns The driver info JSON object.
     */
    const QJsonObject &driverInfo() const;

    /**
     * @brief Returns device resources as a JSON array.
     *
     * For live devices, this returns an empty array. For imported devices,
     * this returns the resources stored in the export file.
     *
     * @returns The resources JSON array.
     */
    const QJsonArray &resources() const;

    /**
     * @brief Returns whether this device was loaded from an export file.
     * @returns @c true if imported, @c false if live.
     */
    bool isImported() const;

    /**
     * @brief Returns whether this device should be considered hidden.
     *
     * Hidden devices are: virtual devices, devices without a driver, or devices without a name.
     * Result is cached at construction time.
     *
     * @returns @c true if the device is hidden, @c false otherwise.
     */
    bool isHidden() const;

    /**
     * @brief Returns whether this device should be displayed in device views.
     * @returns @c true if the device is valid for display, @c false otherwise.
     */
    bool isValidForDisplay() const;

    /**
     * @brief Returns the pre-computed device category.
     * @returns The device category.
     */
    DeviceCategory category() const;

private:
    std::unique_ptr<DeviceInfoPrivate> d_ptr;
    bool isImported_ = false;
};
