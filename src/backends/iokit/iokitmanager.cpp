#include "iokitmanager.h"

#ifdef Q_OS_MACOS

#include <IOKit/IOBSD.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/usb/IOUSBLib.h>

IOKitManager::IOKitManager() = default;

IOKitManager::~IOKitManager() = default;

QString IOKitManager::cfStringToQString(CFStringRef cfStr) {
    if (!cfStr) {
        return {};
    }

    CFIndex length = CFStringGetLength(cfStr);
    if (length == 0) {
        return {};
    }

    CFIndex bufferSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    QByteArray buffer(static_cast<int>(bufferSize), '\0');

    if (CFStringGetCString(cfStr, buffer.data(), bufferSize, kCFStringEncodingUTF8)) {
        return QString::fromUtf8(buffer.constData());
    }

    return {};
}

CFMutableDictionaryRef IOKitManager::getAllProperties(io_service_t service) {
    CFMutableDictionaryRef properties = nullptr;
    kern_return_t result =
        IORegistryEntryCreateCFProperties(service, &properties, kCFAllocatorDefault, 0);
    if (result != KERN_SUCCESS) {
        return nullptr;
    }
    return properties;
}

QString IOKitManager::getStringProperty(io_service_t service, CFStringRef key) {
    CFTypeRef value = IORegistryEntryCreateCFProperty(service, key, kCFAllocatorDefault, 0);
    if (!value) {
        return {};
    }

    QString result;
    CFTypeID typeId = CFGetTypeID(value);

    if (typeId == CFStringGetTypeID()) {
        result = cfStringToQString(static_cast<CFStringRef>(value));
    } else if (typeId == CFDataGetTypeID()) {
        // Some properties are stored as CFData containing a C string
        auto data = static_cast<CFDataRef>(value);
        const UInt8 *bytes = CFDataGetBytePtr(data);
        CFIndex length = CFDataGetLength(data);
        // Remove trailing null if present
        if (length > 0 && bytes[length - 1] == '\0') {
            length--;
        }
        result = QString::fromUtf8(reinterpret_cast<const char *>(bytes), static_cast<int>(length));
    }

    CFRelease(value);
    return result.trimmed();
}

int64_t IOKitManager::getIntProperty(io_service_t service, CFStringRef key) {
    CFTypeRef value = IORegistryEntryCreateCFProperty(service, key, kCFAllocatorDefault, 0);
    if (!value) {
        return 0;
    }

    int64_t result = 0;
    if (CFGetTypeID(value) == CFNumberGetTypeID()) {
        CFNumberGetValue(static_cast<CFNumberRef>(value), kCFNumberSInt64Type, &result);
    }

    CFRelease(value);
    return result;
}

QString IOKitManager::getRegistryPath(io_service_t service) {
    io_string_t path;
    kern_return_t result = IORegistryEntryGetPath(service, kIOServicePlane, path);
    if (result != KERN_SUCCESS) {
        return {};
    }
    return QString::fromUtf8(path);
}

io_service_t IOKitManager::getParent(io_service_t service, const io_name_t plane) {
    io_service_t parent = 0;
    kern_return_t result = IORegistryEntryGetParentEntry(service, plane, &parent);
    if (result != KERN_SUCCESS) {
        return 0;
    }
    return parent;
}

QString IOKitManager::getClassName(io_service_t service) {
    io_name_t className;
    kern_return_t result = IOObjectGetClass(service, className);
    if (result != KERN_SUCCESS) {
        return {};
    }
    return QString::fromUtf8(className);
}

QString IOKitManager::getMatchedDriver(io_service_t service) {
    // Try to get the IOMatchedBundleIdentifier (the kext that matched this device)
    QString driver = getStringProperty(service, CFSTR("IOMatchedPersonality"));
    if (driver.isEmpty()) {
        // Try CFBundleIdentifier from the matched personality
        CFTypeRef personality = IORegistryEntryCreateCFProperty(
            service, CFSTR("IOMatchedPersonality"), kCFAllocatorDefault, 0);
        if (personality && CFGetTypeID(personality) == CFDictionaryGetTypeID()) {
            auto dict = static_cast<CFDictionaryRef>(personality);
            CFStringRef bundleId =
                static_cast<CFStringRef>(CFDictionaryGetValue(dict, CFSTR("CFBundleIdentifier")));
            if (bundleId) {
                driver = cfStringToQString(bundleId);
            }
            CFRelease(personality);
        } else if (personality) {
            CFRelease(personality);
        }
    }

    // Fallback: try to get the provider class as a hint
    if (driver.isEmpty()) {
        driver = getClassName(service);
    }

    return driver;
}

QString IOKitManager::getBSDName(io_service_t service) {
    return getStringProperty(service, CFSTR(kIOBSDNameKey));
}

QString IOKitManager::getProductName(io_service_t service) {
    // Try multiple property keys that might contain the product name
    static const CFStringRef productKeys[] = {
        CFSTR("Product Name"),
        CFSTR("USB Product Name"),
        CFSTR(kIOHIDProductKey),
        CFSTR("Model"),
        CFSTR("device_type"),
        CFSTR("IOName"),
    };

    for (auto key : productKeys) {
        QString name = getStringProperty(service, key);
        if (!name.isEmpty()) {
            return name;
        }
    }

    // Fallback to the IORegistry name
    io_name_t name;
    kern_return_t result = IORegistryEntryGetName(service, name);
    if (result == KERN_SUCCESS && name[0] != '\0') {
        return QString::fromUtf8(name);
    }

    return {};
}

QString IOKitManager::getVendorName(io_service_t service) {
    static const CFStringRef vendorKeys[] = {
        CFSTR("USB Vendor Name"),
        CFSTR(kIOHIDManufacturerKey),
        CFSTR("Manufacturer"),
        CFSTR("vendor-id"),
    };

    for (auto key : vendorKeys) {
        QString vendor = getStringProperty(service, key);
        if (!vendor.isEmpty()) {
            return vendor;
        }
    }

    return {};
}

void IOKitManager::enumerateAllDevices(std::function<void(io_service_t)> callback) const {
    // Enumerate all services in the IOService plane
    io_iterator_t iterator;
    kern_return_t result =
        IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOService"), &iterator);

    if (result != KERN_SUCCESS) {
        return;
    }

    io_service_t service;
    while ((service = IOIteratorNext(iterator)) != 0) {
        callback(service);
        IOObjectRelease(service);
    }

    IOObjectRelease(iterator);
}

void IOKitManager::enumerateDevicesOfClass(const char *className,
                                           std::function<void(io_service_t)> callback) const {
    io_iterator_t iterator;
    kern_return_t result =
        IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching(className), &iterator);

    if (result != KERN_SUCCESS) {
        return;
    }

    io_service_t service;
    while ((service = IOIteratorNext(iterator)) != 0) {
        callback(service);
        IOObjectRelease(service);
    }

    IOObjectRelease(iterator);
}

#endif // Q_OS_MACOS
