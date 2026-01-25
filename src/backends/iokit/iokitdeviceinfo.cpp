#include <QtCore/QDebug>

#include "deviceinfo.h"
#include "iokitdeviceinfo.h"
#include "iokitmanager.h"

IOKitDeviceInfoPrivate::IOKitDeviceInfoPrivate(io_service_t service) : DeviceInfoPrivate() {
    if (!service) {
        isHidden_ = true;
        return;
    }

    extractIOKitProperties(service);
    setNameFromIOKit(service);
    calculateIsHidden();
    calculateCategory();
}

IOKitDeviceInfoPrivate::IOKitDeviceInfoPrivate(const IOKitDeviceInfoPrivate &other, DeviceInfo *q)
    : DeviceInfoPrivate(other, q) {
}

void IOKitDeviceInfoPrivate::extractIOKitProperties(io_service_t service) {
    // Get IORegistry path (equivalent to syspath)
    syspath_ = IOKitManager::getRegistryPath(service);

    // Get parent path
    io_service_t parent = IOKitManager::getParent(service);
    if (parent) {
        parentSyspath_ = IOKitManager::getRegistryPath(parent);
        IOObjectRelease(parent);
    }

    // Get IOKit class name (used for subsystem/category classification)
    platformClassName_ = IOKitManager::getClassName(service);
    subsystem_ = platformClassName_;

    // Get driver (matched kext bundle identifier)
    driver_ = IOKitManager::getMatchedDriver(service);

    // Get BSD name (e.g., "disk0", "en0")
    QString bsdName = IOKitManager::getBSDName(service);
    if (!bsdName.isEmpty()) {
        devnode_ = QStringLiteral("/dev/") + bsdName;
    }

    // Get vendor information
    idVendorFromDatabase_ = IOKitManager::getVendorName(service);

    // Get device path (for hidden device detection)
    devPath_ = syspath_;
}

void IOKitDeviceInfoPrivate::setNameFromIOKit(io_service_t service) {
    // Try to get a meaningful name from various IOKit properties
    name_ = IOKitManager::getProductName(service);

    if (name_.isEmpty()) {
        // Fallback to vendor + class name
        QString vendor = IOKitManager::getVendorName(service);
        if (!vendor.isEmpty()) {
            name_ = vendor + QStringLiteral(" ") + platformClassName_;
        } else {
            name_ = platformClassName_;
        }
    }

    // Clean up the name
    name_ = name_.replace(QLatin1Char('_'), QLatin1Char(' ')).trimmed();
}

void IOKitDeviceInfoPrivate::calculateIsHidden() {
    // Hide low-level IOKit internal classes that aren't useful to display
    static const QStringList hiddenClasses = {
        QStringLiteral("IOService"),
        QStringLiteral("IOResources"),
        QStringLiteral("IOPMrootDomain"),
        QStringLiteral("IORegistryEntry"),
        QStringLiteral("IOPlatformDevice"),
        QStringLiteral("AppleARMIODevice"),
        QStringLiteral("IOInterruptController"),
        QStringLiteral("IODTNVRAM"),
        QStringLiteral("IOUserServer"),
    };
    if (hiddenClasses.contains(platformClassName_)) {
        isHidden_ = true;
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

void IOKitDeviceInfoPrivate::calculateCategory() {
    // macOS: Classify based on IOKit class names
    if (platformClassName_.isEmpty()) {
        category_ = DeviceCategory::Unknown;
        return;
    }

    // USB controllers and devices
    if (platformClassName_.contains(QStringLiteral("USB")) ||
        platformClassName_ == QStringLiteral("IOUSBHostDevice") ||
        platformClassName_ == QStringLiteral("IOUSBDevice") ||
        platformClassName_ == QStringLiteral("AppleUSBHostController")) {
        category_ = DeviceCategory::UniversalSerialBusControllers;
        return;
    }

    // HID devices
    if (platformClassName_.contains(QStringLiteral("HID")) ||
        platformClassName_ == QStringLiteral("IOHIDDevice") ||
        platformClassName_ == QStringLiteral("IOHIDInterface")) {
        // Check for specific HID types
        if (name_.contains(QStringLiteral("Keyboard"), Qt::CaseInsensitive)) {
            category_ = DeviceCategory::Keyboards;
            return;
        }
        if (name_.contains(QStringLiteral("Mouse"), Qt::CaseInsensitive) ||
            name_.contains(QStringLiteral("Trackpad"), Qt::CaseInsensitive) ||
            name_.contains(QStringLiteral("Touchpad"), Qt::CaseInsensitive)) {
            category_ = DeviceCategory::MiceAndOtherPointingDevices;
            return;
        }
        category_ = DeviceCategory::HumanInterfaceDevices;
        return;
    }

    // Storage devices
    if (platformClassName_ == QStringLiteral("IOBlockStorageDevice") ||
        platformClassName_ == QStringLiteral("IONVMeController") ||
        platformClassName_ == QStringLiteral("IOAHCIBlockStorageDevice") ||
        platformClassName_.contains(QStringLiteral("StorageDevice"))) {
        category_ = DeviceCategory::DiskDrives;
        return;
    }

    // Storage volumes (partitions)
    if (platformClassName_ == QStringLiteral("IOMedia") ||
        platformClassName_ == QStringLiteral("IOPartitionScheme") ||
        platformClassName_.contains(QStringLiteral("Partition"))) {
        category_ = DeviceCategory::StorageVolumes;
        return;
    }

    // CD/DVD drives
    if (platformClassName_.contains(QStringLiteral("CD")) ||
        platformClassName_.contains(QStringLiteral("DVD")) ||
        platformClassName_ == QStringLiteral("IOCDBlockStorageDevice") ||
        platformClassName_ == QStringLiteral("IODVDBlockStorageDevice")) {
        category_ = DeviceCategory::DvdCdromDrives;
        return;
    }

    // Network adapters
    if (platformClassName_.contains(QStringLiteral("Network")) ||
        platformClassName_ == QStringLiteral("IONetworkInterface") ||
        platformClassName_ == QStringLiteral("IOEthernetInterface") ||
        platformClassName_.contains(QStringLiteral("Ethernet")) ||
        platformClassName_.contains(QStringLiteral("WiFi")) ||
        platformClassName_.contains(QStringLiteral("AirPort"))) {
        category_ = DeviceCategory::NetworkAdapters;
        return;
    }

    // Display adapters / GPU
    if (platformClassName_.contains(QStringLiteral("GPU")) ||
        platformClassName_.contains(QStringLiteral("Graphics")) ||
        platformClassName_ == QStringLiteral("IOAccelerator") ||
        platformClassName_ == QStringLiteral("AppleGPU") ||
        platformClassName_ == QStringLiteral("AGXAccelerator") ||
        platformClassName_.contains(QStringLiteral("Framebuffer"))) {
        category_ = DeviceCategory::DisplayAdapters;
        return;
    }

    // Audio devices
    if (platformClassName_.contains(QStringLiteral("Audio")) ||
        platformClassName_ == QStringLiteral("IOAudioDevice") ||
        platformClassName_ == QStringLiteral("IOAudioEngine") ||
        platformClassName_.contains(QStringLiteral("Sound"))) {
        category_ = DeviceCategory::AudioInputsAndOutputs;
        return;
    }

    // Batteries
    if (platformClassName_.contains(QStringLiteral("Battery")) ||
        platformClassName_ == QStringLiteral("AppleSmartBattery") ||
        platformClassName_.contains(QStringLiteral("Power"))) {
        category_ = DeviceCategory::Batteries;
        return;
    }

    // PCI devices (system devices)
    if (platformClassName_ == QStringLiteral("IOPCIDevice") ||
        platformClassName_.contains(QStringLiteral("PCI"))) {
        category_ = DeviceCategory::SystemDevices;
        return;
    }

    // Storage controllers
    if (platformClassName_.contains(QStringLiteral("AHCI")) ||
        platformClassName_.contains(QStringLiteral("SATA")) ||
        platformClassName_.contains(QStringLiteral("NVMe")) ||
        platformClassName_.contains(QStringLiteral("StorageController"))) {
        category_ = DeviceCategory::StorageControllers;
        return;
    }

    // Thunderbolt
    if (platformClassName_.contains(QStringLiteral("Thunderbolt"))) {
        category_ = DeviceCategory::SystemDevices;
        return;
    }

    category_ = DeviceCategory::Unknown;
}

QString IOKitDeviceInfoPrivate::propertyValue([[maybe_unused]] const char *key) const {
    // On macOS, we cache all needed properties at construction time
    // This method is mainly used for Linux udev properties
    return {};
}

DeviceInfoPrivate *IOKitDeviceInfoPrivate::clone(DeviceInfo *q) const {
    return new IOKitDeviceInfoPrivate(*this, q);
}

void IOKitDeviceInfoPrivate::dump() const {
    qDebug() << "IOKitDeviceInfoPrivate:";
    qDebug() << "  syspath:" << syspath_;
    qDebug() << "  name:" << name_;
    qDebug() << "  driver:" << driver_;
    qDebug() << "  subsystem:" << subsystem_;
    qDebug() << "  devnode:" << devnode_;
    qDebug() << "  ioKitClass:" << platformClassName_;
    qDebug() << "  category:" << static_cast<int>(category_);
    qDebug() << "  isHidden:" << isHidden_;
}

DeviceInfoPrivate *createDeviceInfo(io_service_t service) {
    return new IOKitDeviceInfoPrivate(service);
}
