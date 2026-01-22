#pragma once

#include <QtGlobal>

#ifdef Q_OS_MACOS

#include <QList>
#include <QMap>
#include <QString>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

class DeviceInfo;

class IOKitManager {
public:
    IOKitManager();
    ~IOKitManager();

    // Enumerate all devices in the IORegistry
    void enumerateAllDevices(std::function<void(io_service_t)> callback) const;

    // Enumerate devices matching a specific IOKit class
    void enumerateDevicesOfClass(const char *className,
                                 std::function<void(io_service_t)> callback) const;

    // Get the IORegistry path for a service (equivalent to syspath)
    static QString getRegistryPath(io_service_t service);

    // Get a string property from an IOKit service
    static QString getStringProperty(io_service_t service, CFStringRef key);

    // Get an integer property from an IOKit service
    static int64_t getIntProperty(io_service_t service, CFStringRef key);

    // Get the parent service
    static io_service_t getParent(io_service_t service, const io_name_t plane = kIOServicePlane);

    // Get the IOKit class name of a service
    static QString getClassName(io_service_t service);

    // Get the matched kext bundle identifier (driver)
    static QString getMatchedDriver(io_service_t service);

    // Get the BSD name (e.g., "disk0", "en0")
    static QString getBSDName(io_service_t service);

    // Get the device model/product name
    static QString getProductName(io_service_t service);

    // Get the vendor name
    static QString getVendorName(io_service_t service);

private:
    // Helper to convert CFString to QString
    static QString cfStringToQString(CFStringRef cfStr);

    // Helper to get all properties as a dictionary
    static CFMutableDictionaryRef getAllProperties(io_service_t service);
};

#else

// Stub class for non-macOS platforms
class IOKitManager {
public:
    IOKitManager() = default;
    ~IOKitManager() = default;
};

#endif
