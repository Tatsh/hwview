#pragma once

#include <QList>
#include <QMap>
#include <memory>

#include <libudev.h>

#include "deviceinfo.h"
#include "udevenumerate.h"

/**
 * @brief Manages udev device enumeration on Linux.
 *
 * This class provides an interface to enumerate devices using the udev library. It owns the
 * udev context and provides methods to scan for devices by subsystem or with custom enumeration
 * criteria.
 */
class UdevManager {
public:
    /**
     * @brief Constructs a @c UdevManager and initializes the @c udev context.
     */
    UdevManager();

    /**
     * @brief Destroys the @c UdevManager and releases the  udev context.
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
     * @returns List of @c DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> iterDevicesSubsystem(const char *subsystem) const;

    /**
     * @brief Enumerates devices in a specific subsystem.
     * @param subsystem The subsystem name as a @c QString.
     * @returns List of @c DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> iterDevicesSubsystem(const QString &subsystem) const;

    /**
     * @brief Converts udev enumeration results to @c DeviceInfo objects.
     * @param enumerator The udev enumerator with scan results.
     * @returns List of @c DeviceInfo objects, sorted by name.
     */
    QList<DeviceInfo> convertToDeviceInfo(struct udev_enumerate *enumerator) const;

    /**
     * @brief Scans devices using a raw udev enumerator.
     * @param enumerator The udev enumerator to scan with.
     * @returns List of @c DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> scanDevices(struct udev_enumerate *enumerator) const;

    /**
     * @brief Scans devices using a @c UdevEnumerate wrapper.
     * @param wrapper The @c UdevEnumerate wrapper with filter criteria.
     * @returns List of @c DeviceInfo objects for matching devices.
     */
    QList<DeviceInfo> scanDevices(const std::unique_ptr<UdevEnumerate> &wrapper) const;

private:
    struct udev *ctx;
};
