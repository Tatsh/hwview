#include "setupapimanager.h"

#include <devpkey.h>

SetupApiManager::SetupApiManager() = default;

SetupApiManager::~SetupApiManager() = default;

void SetupApiManager::enumerateAllDevices(
    std::function<void(HDEVINFO, SP_DEVINFO_DATA *)> callback) const {
    // Get all devices (present and not present)
    HDEVINFO devInfo =
        SetupDiGetClassDevsW(nullptr, nullptr, nullptr, DIGCF_ALLCLASSES | DIGCF_PRESENT);

    if (devInfo == INVALID_HANDLE_VALUE) {
        return;
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devInfoData); ++i) {
        callback(devInfo, &devInfoData);
    }

    SetupDiDestroyDeviceInfoList(devInfo);
}

void SetupApiManager::enumerateDevicesOfClass(
    const GUID *classGuid, std::function<void(HDEVINFO, SP_DEVINFO_DATA *)> callback) const {

    auto devInfo = SetupDiGetClassDevsW(classGuid, nullptr, nullptr, DIGCF_PRESENT);

    if (devInfo == INVALID_HANDLE_VALUE) {
        return;
    }

    SP_DEVINFO_DATA devInfoData;
    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInfo(devInfo, i, &devInfoData); ++i) {
        callback(devInfo, &devInfoData);
    }

    SetupDiDestroyDeviceInfoList(devInfo);
}

QString SetupApiManager::getDeviceRegistryProperty(HDEVINFO devInfo,
                                                   SP_DEVINFO_DATA *devInfoData,
                                                   DWORD property) {
    DWORD dataType;
    DWORD requiredSize = 0;

    // First call to get required buffer size
    SetupDiGetDeviceRegistryPropertyW(
        devInfo, devInfoData, property, &dataType, nullptr, 0, &requiredSize);

    if (requiredSize == 0) {
        return {};
    }

    QByteArray buffer(static_cast<int>(requiredSize), '\0');

    if (!SetupDiGetDeviceRegistryPropertyW(devInfo,
                                           devInfoData,
                                           property,
                                           &dataType,
                                           reinterpret_cast<PBYTE>(buffer.data()),
                                           requiredSize,
                                           nullptr)) {
        return {};
    }

    if (dataType == REG_SZ || dataType == REG_EXPAND_SZ) {
        return QString::fromWCharArray(reinterpret_cast<const wchar_t *>(buffer.constData()));
    }

    return {};
}

QString SetupApiManager::getDeviceInstanceId(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    wchar_t instanceId[MAX_DEVICE_ID_LEN];

    if (!SetupDiGetDeviceInstanceIdW(
            devInfo, devInfoData, instanceId, MAX_DEVICE_ID_LEN, nullptr)) {
        return {};
    }

    return QString::fromWCharArray(instanceId);
}

QString SetupApiManager::getParentDeviceInstanceId(SP_DEVINFO_DATA *devInfoData) {
    DEVINST parentDevInst;

    if (CM_Get_Parent(&parentDevInst, devInfoData->DevInst, 0) != CR_SUCCESS) {
        return {};
    }

    wchar_t parentId[MAX_DEVICE_ID_LEN];
    if (CM_Get_Device_IDW(parentDevInst, parentId, MAX_DEVICE_ID_LEN, 0) != CR_SUCCESS) {
        return {};
    }

    return QString::fromWCharArray(parentId);
}

GUID SetupApiManager::getDeviceClassGuid(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    Q_UNUSED(devInfo);
    return devInfoData->ClassGuid;
}

QString SetupApiManager::getDeviceClassName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    Q_UNUSED(devInfo);

    wchar_t className[MAX_CLASS_NAME_LEN];

    if (!SetupDiClassNameFromGuidW(
            &devInfoData->ClassGuid, className, MAX_CLASS_NAME_LEN, nullptr)) {
        return {};
    }

    return QString::fromWCharArray(className);
}

QString SetupApiManager::getDeviceFriendlyName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    // Try friendly name first
    auto name = getDeviceRegistryProperty(devInfo, devInfoData, SPDRP_FRIENDLYNAME);

    if (name.isEmpty()) {
        // Fall back to device description
        name = getDeviceRegistryProperty(devInfo, devInfoData, SPDRP_DEVICEDESC);
    }

    return name;
}

QString SetupApiManager::getDriverKeyName(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    return getDeviceRegistryProperty(devInfo, devInfoData, SPDRP_DRIVER);
}

bool SetupApiManager::getDeviceStatus(SP_DEVINFO_DATA *devInfoData,
                                      ULONG *status,
                                      ULONG *problemCode) {
    return CM_Get_DevNode_Status(status, problemCode, devInfoData->DevInst, 0) == CR_SUCCESS;
}

bool SetupApiManager::isDeviceHidden(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    DWORD dataType;
    DWORD capabilities = 0;
    DWORD requiredSize = sizeof(capabilities);

    if (SetupDiGetDeviceRegistryPropertyW(devInfo,
                                          devInfoData,
                                          SPDRP_CAPABILITIES,
                                          &dataType,
                                          reinterpret_cast<PBYTE>(&capabilities),
                                          requiredSize,
                                          nullptr)) {
        // CM_DEVCAP_SILENTINSTALL = 0x00000004
        // Devices with this capability are typically hidden/internal
        // But we check for the actual hidden registry flag
    }

    // Check ConfigFlags for hidden flag
    DWORD configFlags = 0;
    requiredSize = sizeof(configFlags);
    if (SetupDiGetDeviceRegistryPropertyW(devInfo,
                                          devInfoData,
                                          SPDRP_CONFIGFLAGS,
                                          &dataType,
                                          reinterpret_cast<PBYTE>(&configFlags),
                                          requiredSize,
                                          nullptr)) {
        // CONFIGFLAG_HIDDEN = 0x00000010
        if (configFlags & 0x00000010) {
            return true;
        }
    }

    return false;
}

QStringList SetupApiManager::getHardwareIds(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    DWORD dataType;
    DWORD requiredSize = 0;

    SetupDiGetDeviceRegistryPropertyW(
        devInfo, devInfoData, SPDRP_HARDWAREID, &dataType, nullptr, 0, &requiredSize);

    if (requiredSize == 0) {
        return {};
    }

    QByteArray buffer(static_cast<int>(requiredSize), '\0');

    if (!SetupDiGetDeviceRegistryPropertyW(devInfo,
                                           devInfoData,
                                           SPDRP_HARDWAREID,
                                           &dataType,
                                           reinterpret_cast<PBYTE>(buffer.data()),
                                           requiredSize,
                                           nullptr)) {
        return {};
    }

    if (dataType == REG_MULTI_SZ) {
        return multiSzToStringList(reinterpret_cast<const wchar_t *>(buffer.constData()));
    }

    return {};
}

QString SetupApiManager::getManufacturer(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    return getDeviceRegistryProperty(devInfo, devInfoData, SPDRP_MFG);
}

QString SetupApiManager::getPhysicalDeviceObjectName(HDEVINFO devInfo,
                                                     SP_DEVINFO_DATA *devInfoData) {
    return getDeviceRegistryProperty(devInfo, devInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);
}

QStringList SetupApiManager::multiSzToStringList(const wchar_t *multiSz) {
    QStringList result;

    if (!multiSz) {
        return result;
    }

    const wchar_t *current = multiSz;
    while (*current) {
        result << QString::fromWCharArray(current);
        current += wcslen(current) + 1;
    }

    return result;
}
