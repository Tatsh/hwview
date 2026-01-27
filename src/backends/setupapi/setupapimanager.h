// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <functional>

#include <QtCore/QList>
#include <QtCore/QString>

#include <windows.h>

#include <cfgmgr32.h>
#include <setupapi.h>

class DeviceInfo;

/**
 * @brief Manages SetupAPI device enumeration on Windows.
 *
 * This class provides an interface to enumerate devices using the Windows SetupAPI. It provides
 * methods to iterate over devices and extract device properties from the registry.
 */
class SetupApiManager {
public:
    /**
     * @brief Constructs a @c SetupApiManager.
     */
    SetupApiManager();

    /**
     * @brief Destroys the @c SetupApiManager.
     */
    ~SetupApiManager();

    /**
     * @brief Enumerates all devices in the system.
     * @param callback Function called for each device found.
     */
    void enumerateAllDevices(std::function<void(HDEVINFO, SP_DEVINFO_DATA *)> callback) const;

    /**
     * @brief Enumerates devices of a specific class.
     * @param classGuid Pointer to the device class GUID.
     * @param callback Function called for each matching device.
     */
    void enumerateDevicesOfClass(const GUID *classGuid,
                                 std::function<void(HDEVINFO, SP_DEVINFO_DATA *)> callback) const;

    /**
     * @brief Gets a string property from a device.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @param property The @c SPDRP_* property constant.
     * @returns The property value, or empty string if not found.
     */
    static QString
    getDeviceRegistryProperty(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData, DWORD property);

    /**
     * @brief Gets the device instance ID (equivalent to syspath).
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns The device instance ID string.
     */
    static QString getDeviceInstanceId(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the parent device instance ID.
     * @param devInfoData Pointer to the device info data.
     * @returns The parent device instance ID.
     */
    static QString getParentDeviceInstanceId(SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the device class GUID.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns The device class GUID.
     */
    static GUID getDeviceClassGuid(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the device class name.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns The device class name.
     */
    static QString getDeviceClassName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the device friendly name or description.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns The friendly name, or description as fallback.
     */
    static QString getDeviceFriendlyName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the driver key name.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns The driver registry key name.
     */
    static QString getDriverKeyName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the device status and problem code.
     * @param devInfoData Pointer to the device info data.
     * @param status Output parameter for device status flags.
     * @param problemCode Output parameter for problem code if any.
     * @returns @c true if status was retrieved successfully.
     */
    static bool getDeviceStatus(SP_DEVINFO_DATA *devInfoData, ULONG *status, ULONG *problemCode);

    /**
     * @brief Checks if a device is marked as hidden.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns @c true if the device is hidden, @c false otherwise.
     */
    static bool isDeviceHidden(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the hardware IDs for a device.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns List of hardware ID strings.
     */
    static QStringList getHardwareIds(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the manufacturer name.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns The manufacturer name.
     */
    static QString getManufacturer(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    /**
     * @brief Gets the physical device object name.
     * @param devInfo The device information set handle.
     * @param devInfoData Pointer to the device info data.
     * @returns The PDO name.
     */
    static QString getPhysicalDeviceObjectName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

private:
    static QStringList multiSzToStringList(const wchar_t *multiSz);
};
