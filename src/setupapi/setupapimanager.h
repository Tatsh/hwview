#pragma once

#include <QtGlobal>

#ifdef Q_OS_WIN

#include <QList>
#include <QString>
#include <functional>

#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>

class DeviceInfo;

class SetupApiManager {
public:
    SetupApiManager();
    ~SetupApiManager();

    // Enumerate all devices, calling the callback for each one
    void enumerateAllDevices(std::function<void(HDEVINFO, SP_DEVINFO_DATA *)> callback) const;

    // Enumerate devices of a specific class
    void enumerateDevicesOfClass(const GUID *classGuid,
                                  std::function<void(HDEVINFO, SP_DEVINFO_DATA *)> callback) const;

    // Get a string property from a device using SPDRP_* constants
    static QString getDeviceRegistryProperty(HDEVINFO devInfo,
                                             SP_DEVINFO_DATA *devInfoData,
                                             DWORD property);

    // Get the device instance ID (equivalent to syspath)
    static QString getDeviceInstanceId(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get the parent device instance ID
    static QString getParentDeviceInstanceId(SP_DEVINFO_DATA *devInfoData);

    // Get the device class GUID
    static GUID getDeviceClassGuid(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get the device class name
    static QString getDeviceClassName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get the device friendly name (or description as fallback)
    static QString getDeviceFriendlyName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get the driver key name
    static QString getDriverKeyName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get device status (working, has problem, etc.)
    static bool getDeviceStatus(SP_DEVINFO_DATA *devInfoData, ULONG *status, ULONG *problemCode);

    // Check if device is hidden
    static bool isDeviceHidden(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get the hardware IDs for a device
    static QStringList getHardwareIds(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get manufacturer
    static QString getManufacturer(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

    // Get physical device object name
    static QString getPhysicalDeviceObjectName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData);

private:
    // Helper to convert multi-sz string to QStringList
    static QStringList multiSzToStringList(const wchar_t *multiSz);
};

#else

// Stub class for non-Windows platforms
class SetupApiManager {
public:
    SetupApiManager() = default;
    ~SetupApiManager() = default;
};

#endif // Q_OS_WIN
