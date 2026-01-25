#pragma once

#include "deviceinfo_p.h"

#include <libudev.h>

/**
 * @brief Linux udev implementation of DeviceInfoPrivate.
 */
class UdevDeviceInfoPrivate : public DeviceInfoPrivate {
public:
    /**
     * @brief Construct from a udev device path.
     * @param ctx The udev context.
     * @param syspath The system path to the device.
     */
    UdevDeviceInfoPrivate(udev *ctx, const char *syspath);

    ~UdevDeviceInfoPrivate() override;

    QString propertyValue(const char *key) const override;
    DeviceInfoPrivate *clone(DeviceInfo *q) const override;
    void dump() const override;

private:
    // Copy constructor for clone()
    UdevDeviceInfoPrivate(const UdevDeviceInfoPrivate &other, DeviceInfo *q);

    void setName();
    void calculateIsHidden();
    void calculateCategory();

    udev *ctx_;
    udev_device *dev_;
};

/**
 * @brief Factory function to create a DeviceInfoPrivate from udev.
 * @param ctx The udev context.
 * @param syspath The device system path.
 * @returns A new UdevDeviceInfoPrivate instance (q_ptr will be set by DeviceInfo).
 */
DeviceInfoPrivate *createDeviceInfo(udev *ctx, const char *syspath);
