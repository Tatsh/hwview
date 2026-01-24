#pragma once

#include <QtGlobal>

#ifdef Q_OS_MACOS

#include <QList>
#include <QMap>
#include <QString>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

class DeviceInfo;

/**
 * @brief Manages IOKit device enumeration on macOS.
 *
 * This class provides an interface to enumerate devices using Apple's
 * IOKit framework. It provides methods to iterate over devices in the
 * IORegistry and extract device properties.
 */
class IOKitManager {
public:
    /**
     * @brief Constructs an IOKitManager.
     */
    IOKitManager();

    /**
     * @brief Destroys the IOKitManager.
     */
    ~IOKitManager();

    /**
     * @brief Enumerates all devices in the IORegistry.
     * @param callback Function called for each device service found.
     */
    void enumerateAllDevices(std::function<void(io_service_t)> callback) const;

    /**
     * @brief Enumerates devices matching a specific IOKit class.
     * @param className The IOKit class name to match.
     * @param callback Function called for each matching service.
     */
    void enumerateDevicesOfClass(const char *className,
                                 std::function<void(io_service_t)> callback) const;

    /**
     * @brief Gets the IORegistry path for a service.
     * @param service The IOKit service object.
     * @returns The registry path string.
     */
    static QString getRegistryPath(io_service_t service);

    /**
     * @brief Gets a string property from an IOKit service.
     * @param service The IOKit service object.
     * @param key The property key as a CFString.
     * @returns The property value, or empty string if not found.
     */
    static QString getStringProperty(io_service_t service, CFStringRef key);

    /**
     * @brief Gets an integer property from an IOKit service.
     * @param service The IOKit service object.
     * @param key The property key as a CFString.
     * @returns The property value, or 0 if not found.
     */
    static int64_t getIntProperty(io_service_t service, CFStringRef key);

    /**
     * @brief Gets the parent service in the IORegistry.
     * @param service The IOKit service object.
     * @param plane The IORegistry plane to traverse.
     * @returns The parent service, or IO_OBJECT_NULL if none.
     */
    static io_service_t getParent(io_service_t service, const io_name_t plane = kIOServicePlane);

    /**
     * @brief Gets the IOKit class name of a service.
     * @param service The IOKit service object.
     * @returns The class name string.
     */
    static QString getClassName(io_service_t service);

    /**
     * @brief Gets the matched kext bundle identifier (driver).
     * @param service The IOKit service object.
     * @returns The driver bundle identifier.
     */
    static QString getMatchedDriver(io_service_t service);

    /**
     * @brief Gets the BSD name (e.g., "disk0", "en0").
     * @param service The IOKit service object.
     * @returns The BSD name, or empty string if none.
     */
    static QString getBSDName(io_service_t service);

    /**
     * @brief Gets the device model/product name.
     * @param service The IOKit service object.
     * @returns The product name.
     */
    static QString getProductName(io_service_t service);

    /**
     * @brief Gets the vendor name.
     * @param service The IOKit service object.
     * @returns The vendor name.
     */
    static QString getVendorName(io_service_t service);

private:
    static QString cfStringToQString(CFStringRef cfStr);
    static CFMutableDictionaryRef getAllProperties(io_service_t service);
};

#else

/**
 * @brief Stub class for IOKitManager on non-macOS platforms.
 */
class IOKitManager {
public:
    IOKitManager() = default;
    ~IOKitManager() = default;
};

#endif
