#pragma once

#include <QList>
#include <QMap>
#include <memory>

#include "deviceinfo.h"

#ifdef Q_OS_LINUX
#include "udev/udevenumerate.h"
#include <libudev.h>

/**
 * @brief Manages udev device enumeration on Linux.
 *
 * This class provides an interface to enumerate devices using the udev
 * library. It owns the udev context and provides methods to scan for
 * devices by subsystem or with custom enumeration criteria.
 */
class UdevManager {
public:
    /**
     * @brief Constructs a UdevManager and initializes the udev context.
     */
    UdevManager();

    /**
     * @brief Destroys the UdevManager and releases the udev context.
     */
    ~UdevManager();

    /**
     * @brief Returns the udev context.
     * @returns Pointer to the udev context structure.
     */
    struct udev *context() const;

    /**
     * @brief Enumerates devices in a specific subsystem.
     * @param subsystem The subsystem name (e.g., "usb", "pci", "block").
     * @returns List of DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> iterDevicesSubsystem(const char *subsystem) const;

    /**
     * @brief Enumerates devices in a specific subsystem.
     * @param subsystem The subsystem name as a QString.
     * @returns List of DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> iterDevicesSubsystem(const QString &subsystem) const;

    /**
     * @brief Converts udev enumeration results to DeviceInfo objects.
     * @param enumerator The udev enumerator with scan results.
     * @returns List of DeviceInfo objects, sorted by name.
     */
    QList<DeviceInfo> convertToDeviceInfo(struct udev_enumerate *enumerator) const;

    /**
     * @brief Scans devices using a raw udev enumerator.
     * @param enumerator The udev enumerator to scan with.
     * @returns List of DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> scanDevices(struct udev_enumerate *enumerator) const;

    /**
     * @brief Scans devices using a UdevEnumerate wrapper.
     * @param wrapper The UdevEnumerate wrapper with filter criteria.
     * @returns List of DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> scanDevices(const std::unique_ptr<UdevEnumerate> &wrapper) const;

private:
    struct udev *ctx;
};
#else
/**
 * @brief Stub class for UdevManager on non-Linux platforms.
 */
class UdevManager {
public:
    UdevManager() = default;
    ~UdevManager() = default;

    /**
     * @brief Stub that returns nullptr.
     * @returns Always returns nullptr on non-Linux platforms.
     */
    void *context() const {
        return nullptr;
    }

    /**
     * @brief Stub that returns an empty list.
     * @param subsystem Ignored.
     * @returns Empty list.
     */
    QList<DeviceInfo> iterDevicesSubsystem(const char *subsystem) const {
        Q_UNUSED(subsystem);
        return {};
    }

    /**
     * @brief Stub that returns an empty list.
     * @param subsystem Ignored.
     * @returns Empty list.
     */
    QList<DeviceInfo> iterDevicesSubsystem(const QString &subsystem) const {
        Q_UNUSED(subsystem);
        return {};
    }
};
#endif
