#include "setupapideviceinfo.h"
#include "devclass_guids.h"
#include "deviceinfo.h"
#include "setupapimanager.h"

#include <QDebug>

SetupApiDeviceInfoPrivate::SetupApiDeviceInfoPrivate(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData)
    : DeviceInfoPrivate() {
    if (!devInfo || devInfo == INVALID_HANDLE_VALUE || !devInfoData) {
        isHidden_ = true;
        return;
    }

    extractWindowsProperties(devInfo, devInfoData);
    calculateIsHidden();
    calculateCategory();
}

SetupApiDeviceInfoPrivate::SetupApiDeviceInfoPrivate(const SetupApiDeviceInfoPrivate &other,
                                                     DeviceInfo *q)
    : DeviceInfoPrivate(other, q) {
}

void SetupApiDeviceInfoPrivate::extractWindowsProperties(HDEVINFO devInfo,
                                                         SP_DEVINFO_DATA *devInfoData) {
    // Get device instance ID (equivalent to syspath)
    syspath_ = SetupApiManager::getDeviceInstanceId(devInfo, devInfoData);

    // Get parent device ID
    parentSyspath_ = SetupApiManager::getParentDeviceInstanceId(devInfoData);

    // Get device class name (equivalent to subsystem)
    platformClassName_ = SetupApiManager::getDeviceClassName(devInfo, devInfoData);
    subsystem_ = platformClassName_;

    // Get driver key name
    driver_ = SetupApiManager::getDriverKeyName(devInfo, devInfoData);

    // Get friendly name or description
    name_ = SetupApiManager::getDeviceFriendlyName(devInfo, devInfoData);

    // Get manufacturer
    idVendorFromDatabase_ = SetupApiManager::getManufacturer(devInfo, devInfoData);

    // Get physical device object name
    devnode_ = SetupApiManager::getPhysicalDeviceObjectName(devInfo, devInfoData);

    // Use instance ID as device path
    devPath_ = syspath_;

    // Get device class GUID for category classification
    GUID classGuid = SetupApiManager::getDeviceClassGuid(devInfo, devInfoData);
    category_ = setupapi::classGuidToCategory(classGuid);

    // Check if device is marked as hidden
    isHidden_ = SetupApiManager::isDeviceHidden(devInfo, devInfoData);
}

void SetupApiDeviceInfoPrivate::calculateIsHidden() {
    // isHidden_ was already set from ConfigFlags during property extraction
    if (isHidden_) {
        return;
    }

    if (driver_.isEmpty()) {
        isHidden_ = true;
        return;
    }
    if (name_.isEmpty()) {
        isHidden_ = true;
        return;
    }
    isHidden_ = false;
}

void SetupApiDeviceInfoPrivate::calculateCategory() {
    // Category was already set from device class GUID during property extraction
    // If it's still Unknown, try to determine from class name
    if (category_ == DeviceCategory::Unknown && !platformClassName_.isEmpty()) {
        if (platformClassName_.compare(QStringLiteral("USB"), Qt::CaseInsensitive) == 0) {
            category_ = DeviceCategory::UniversalSerialBusControllers;
        } else if (platformClassName_.compare(QStringLiteral("DiskDrive"), Qt::CaseInsensitive) ==
                   0) {
            category_ = DeviceCategory::DiskDrives;
        } else if (platformClassName_.compare(QStringLiteral("Display"), Qt::CaseInsensitive) ==
                   0) {
            category_ = DeviceCategory::DisplayAdapters;
        } else if (platformClassName_.compare(QStringLiteral("Net"), Qt::CaseInsensitive) == 0) {
            category_ = DeviceCategory::NetworkAdapters;
        } else if (platformClassName_.compare(QStringLiteral("Keyboard"), Qt::CaseInsensitive) ==
                   0) {
            category_ = DeviceCategory::Keyboards;
        } else if (platformClassName_.compare(QStringLiteral("Mouse"), Qt::CaseInsensitive) == 0) {
            category_ = DeviceCategory::MiceAndOtherPointingDevices;
        } else if (platformClassName_.compare(QStringLiteral("HIDClass"), Qt::CaseInsensitive) ==
                   0) {
            category_ = DeviceCategory::HumanInterfaceDevices;
        } else if (platformClassName_.compare(QStringLiteral("Volume"), Qt::CaseInsensitive) == 0) {
            category_ = DeviceCategory::StorageVolumes;
        } else if (platformClassName_.compare(QStringLiteral("System"), Qt::CaseInsensitive) == 0) {
            category_ = DeviceCategory::SystemDevices;
        }
    }
}

QString SetupApiDeviceInfoPrivate::propertyValue([[maybe_unused]] const char *key) const {
    // On Windows, we cache all needed properties at construction time
    return {};
}

DeviceInfoPrivate *SetupApiDeviceInfoPrivate::clone(DeviceInfo *q) const {
    return new SetupApiDeviceInfoPrivate(*this, q);
}

void SetupApiDeviceInfoPrivate::dump() const {
    qDebug() << "SetupApiDeviceInfoPrivate:";
    qDebug() << "  syspath:" << syspath_;
    qDebug() << "  name:" << name_;
    qDebug() << "  driver:" << driver_;
    qDebug() << "  subsystem:" << subsystem_;
    qDebug() << "  devnode:" << devnode_;
    qDebug() << "  deviceClass:" << platformClassName_;
    qDebug() << "  category:" << static_cast<int>(category_);
    qDebug() << "  isHidden:" << isHidden_;
}

DeviceInfoPrivate *createDeviceInfo(HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
    return new SetupApiDeviceInfoPrivate(devInfo, devInfoData);
}
