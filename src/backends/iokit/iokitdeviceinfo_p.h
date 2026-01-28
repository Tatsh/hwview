// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include "deviceinfo_p.h"

#include <IOKit/IOKitLib.h>

/**
 * @brief macOS IOKit implementation of DeviceInfoPrivate.
 */
class IOKitDeviceInfoPrivate : public DeviceInfoPrivate {
public:
    /**
     * @brief Construct from an IOKit service.
     * @param service The IOKit service object.
     */
    explicit IOKitDeviceInfoPrivate(io_service_t service);

    ~IOKitDeviceInfoPrivate() override = default;

    QString propertyValue(const char *key) const override;
    DeviceInfoPrivate *clone(DeviceInfo *q) const override;
    void dump() const override;

private:
    // Copy constructor for clone()
    IOKitDeviceInfoPrivate(const IOKitDeviceInfoPrivate &other, DeviceInfo *q);

    void extractIOKitProperties(io_service_t service);
    void setNameFromIOKit(io_service_t service);
    void calculateIsHidden();
    void calculateCategory();
};

/**
 * @brief Factory function to create a DeviceInfoPrivate from IOKit.
 * @param service The IOKit service object.
 * @returns A new IOKitDeviceInfoPrivate instance (q_ptr will be set by DeviceInfo).
 */
DeviceInfoPrivate *createDeviceInfo(io_service_t service);
