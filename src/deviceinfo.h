#pragma once

#include <QString>

#ifdef Q_OS_LINUX
#include <libudev.h>
#elif defined(Q_OS_MACOS)
#include <IOKit/IOKitLib.h>
#elif defined(Q_OS_WIN)
#include <setupapi.h>
#include <windows.h>
#else
// Stub types for unsupported platforms
struct udev;
struct udev_device;
#endif

/**
 * @brief Pre-computed device category for fast classification.
 *
 * Devices are categorized at construction time to avoid repeated classification logic during
 * display.
 */
enum class DeviceCategory {
    Unknown,                       ///< Unknown or unclassified device
    AudioInputsAndOutputs,         ///< Audio input/output devices
    Batteries,                     ///< Battery and power supply devices
    Computer,                      ///< The computer itself
    DiskDrives,                    ///< Hard disk and solid state drives
    DisplayAdapters,               ///< Graphics cards and display adapters
    DvdCdromDrives,                ///< Optical disc drives
    HumanInterfaceDevices,         ///< Generic HID devices
    Keyboards,                     ///< Keyboard devices
    MiceAndOtherPointingDevices,   ///< Mice, touchpads, and pointing devices
    NetworkAdapters,               ///< Network interface cards
    SoftwareDevices,               ///< Virtual and software-based devices
    SoundVideoAndGameControllers,  ///< Multimedia controllers
    StorageControllers,            ///< Storage host controllers
    StorageVolumes,                ///< Disk partitions and volumes
    SystemDevices,                 ///< System and bridge devices
    UniversalSerialBusControllers, ///< USB host controllers
};

/**
 * @brief Encapsulates information about a hardware device.
 *
 * This class provides a cross-platform abstraction for device information. On Linux, it wraps udev
 * device data. On macOS, it wraps IOKit data. On Windows, it wraps SetupAPI data.
 *
 * DeviceInfo objects are copyable and movable. On Linux, copying uses udev reference counting for
 * efficient memory management.
 */
class DeviceInfo {
public:
#ifdef Q_OS_LINUX
    /**
     * @brief Constructs a DeviceInfo from a udev device path.
     * @param ctx The udev context.
     * @param syspath The system path to the device (e.g., "/sys/devices/...").
     */
    DeviceInfo(udev *ctx, const char *syspath);
#elif defined(Q_OS_MACOS)
    /**
     * @brief Constructs a DeviceInfo from an IOKit service.
     * @param service The IOKit service object.
     */
    explicit DeviceInfo(io_service_t service);
#elif defined(Q_OS_WIN)
    /**
     * @brief Constructs a DeviceInfo from SetupAPI device info.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data structure.
     */
    DeviceInfo(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);
#endif
    ~DeviceInfo();

    /**
     * @brief Copy constructor.
     * @param other The DeviceInfo to copy from.
     */
    DeviceInfo(const DeviceInfo &other);

    /**
     * @brief Copy assignment operator.
     * @param other The DeviceInfo to copy from.
     * @returns Reference to this object.
     */
    DeviceInfo &operator=(const DeviceInfo &other);

    /**
     * @brief Move constructor.
     * @param other The DeviceInfo to move from.
     */
    DeviceInfo(DeviceInfo &&other) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other The DeviceInfo to move from.
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
     * @brief Returns a udev property value.
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
     * @brief Returns the cached DEVPATH property value.
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
     * @brief Returns the cached ID_CDROM property.
     * @returns "1" if this is a CD-ROM device, empty otherwise.
     */
    const QString &idCdrom() const;

    /**
     * @brief Returns the cached DEVTYPE property.
     * @returns The device type (e.g., "disk", "partition").
     */
    const QString &devType() const;

    /**
     * @brief Returns the cached ID_INPUT_KEYBOARD property.
     * @returns "1" if this is a keyboard, empty otherwise.
     */
    const QString &idInputKeyboard() const;

    /**
     * @brief Returns the cached ID_INPUT_MOUSE property.
     * @returns "1" if this is a mouse, empty otherwise.
     */
    const QString &idInputMouse() const;

    /**
     * @brief Returns the cached ID_TYPE property.
     * @returns The ID type (e.g., "audio", "disk").
     */
    const QString &idType() const;

    /**
     * @brief Returns the cached model name from hardware database.
     * @returns The model name from udev database.
     */
    const QString &idModelFromDatabase() const;

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
#ifdef Q_OS_LINUX
    void setName();

    udev *ctx;
    udev_device *dev;
#elif defined(Q_OS_MACOS)
    void setNameFromIOKit(io_service_t service);
    void extractIOKitProperties(io_service_t service);

    QString ioKitClassName_;
#elif defined(Q_OS_WIN)
    void extractWindowsProperties(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    QString deviceClassName_;
#endif
    void calculateIsHidden();
    void calculateCategory();

    QString devPath_;
    QString driver_;
    QString hidID_;
    QString hidName_;
    QString hidPhysicalMac_;
    QString hidUniq_;
    QString modAlias_;
    QString name_;
    QString subsystem_;
    QString syspath_;
    QString parentSyspath_;
    QString devnode_;
    QString idVendorFromDatabase_;
    QString pciClass_;
    QString pciSubclass_;
    QString pciInterface_;
    QString idCdrom_;
    QString devType_;
    QString idInputKeyboard_;
    QString idInputMouse_;
    QString idType_;
    QString idModelFromDatabase_;
    bool isHidden_ = false;
    DeviceCategory category_ = DeviceCategory::Unknown;
};
