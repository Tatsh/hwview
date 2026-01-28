// SPDX-License-Identifier: MIT
#pragma once

#include "deviceinfo_p.h"

#include <setupapi.h>
#include <windows.h>

/**
 * @brief Windows SetupAPI implementation of DeviceInfoPrivate.
 */
class SetupApiDeviceInfoPrivate : public DeviceInfoPrivate {
public:
    /**
     * @brief Construct from SetupAPI device info.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data structure.
     */
    SetupApiDeviceInfoPrivate(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    ~SetupApiDeviceInfoPrivate() override = default;

    QString propertyValue(const char *key) const override;
    DeviceInfoPrivate *clone(DeviceInfo *q) const override;
    void dump() const override;

private:
    // Copy constructor for clone()
    SetupApiDeviceInfoPrivate(const SetupApiDeviceInfoPrivate &other, DeviceInfo *q);

    void extractWindowsProperties(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);
    void calculateIsHidden();
    void calculateCategory();
};

/**
 * @brief Factory function to create a DeviceInfoPrivate from SetupAPI.
 * @param devInfo The device information set handle.
 * @param devInfoData Pointer to the device info data structure.
 * @returns A new SetupApiDeviceInfoPrivate instance (q_ptr will be set by DeviceInfo).
 */
DeviceInfoPrivate *createDeviceInfo(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);
