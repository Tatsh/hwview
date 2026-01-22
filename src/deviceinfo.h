#pragma once

#include <QString>

#ifdef Q_OS_LINUX
#include <libudev.h>
#else
// Stub types for non-Linux platforms
struct udev;
struct udev_device;
#endif

// Pre-computed device category for fast classification
enum class DeviceCategory {
    Unknown,
    AudioInputsAndOutputs,
    Batteries,
    Computer,
    DiskDrives,
    DisplayAdapters,
    DvdCdromDrives,
    HumanInterfaceDevices,
    Keyboards,
    MiceAndOtherPointingDevices,
    NetworkAdapters,
    SoftwareDevices,
    SoundVideoAndGameControllers,
    StorageControllers,
    StorageVolumes,
    SystemDevices,
    UniversalSerialBusControllers,
};

class DeviceInfo {
public:
#ifdef Q_OS_LINUX
    DeviceInfo(udev *, const char *);
#endif
    ~DeviceInfo();

    // Copy operations (uses udev_device_ref for reference counting)
    DeviceInfo(const DeviceInfo &other);
    DeviceInfo &operator=(const DeviceInfo &other);

    // Move operations (udev_device ownership transfer)
    DeviceInfo(DeviceInfo &&other) noexcept;
    DeviceInfo &operator=(DeviceInfo &&other) noexcept;

    const QString &driver() const;
    const QString &name() const;
    QString propertyValue(const char *) const;
    const QString &syspath() const;
    const QString &parentSyspath() const;
    const QString &subsystem() const;
    const QString &devnode() const;
    void dump();

    // Cached property accessors for frequently-used values (avoid repeated syscalls)
    const QString &devPath() const;
    const QString &pciClass() const;
    const QString &pciSubclass() const;
    const QString &pciInterface() const;
    const QString &idCdrom() const;
    const QString &devType() const;
    const QString &idInputKeyboard() const;
    const QString &idInputMouse() const;
    const QString &idType() const;
    const QString &idModelFromDatabase() const;

    // Returns true if this device should be considered "hidden"
    // Hidden devices are: virtual devices, devices without a driver, or devices without a name
    // Result is cached at construction time for performance
    bool isHidden() const;

    // Returns true if this device matches criteria for display in device views
    // Used by multiple models to filter devices consistently
    bool isValidForDisplay() const;

    // Pre-computed device category for fast classification
    DeviceCategory category() const;

private:
#ifdef Q_OS_LINUX
    void setName();

    udev *ctx;
    udev_device *dev;
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
    // Cached frequently-accessed properties
    QString pciClass_;
    QString pciSubclass_;
    QString pciInterface_;
    QString idCdrom_;
    QString devType_;
    QString idInputKeyboard_;
    QString idInputMouse_;
    QString idType_;
    QString idModelFromDatabase_;
    bool isHidden_ = false;                             // Cached result
    DeviceCategory category_ = DeviceCategory::Unknown; // Cached category
};
