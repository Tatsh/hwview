#pragma once

#include <QHash>
#include <QList>
#include <QMutex>
#include <QObject>

#include "deviceinfo.h"

#ifdef Q_OS_LINUX
#include "udev/udevmanager.h"
#include "udev/udevmonitor.h"
// Forward declaration for udev context
struct udev;
#elif defined(Q_OS_MACOS)
#include "iokit/iokitmanager.h"
#elif defined(Q_OS_WIN)
#include "setupapi/setupapimanager.h"
#endif

/**
 * @brief Singleton cache that holds all device information.
 *
 * This class enumerates devices once at startup and caches them for reuse by all models. It
 * provides thread-safe access to device information and supports automatic monitoring for device
 * changes on Linux via udev.
 *
 * The cache can be manually refreshed by calling refresh(), or it will automatically refresh when
 * devices are added or removed (on Linux with udev monitoring enabled).
 *
 * @note This is a singleton class. Use instance() to access the single instance.
 *
 * Example usage:
 * @code
 * // Get all devices
 * auto devices = DeviceCache::instance().allDevices();
 *
 * // Connect to device change notifications
 * connect(&DeviceCache::instance(), &DeviceCache::devicesChanged,
 *         this, &MyClass::onDevicesChanged);
 * @endcode
 */
class DeviceCache : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Returns the singleton instance of the device cache.
     * @returns Reference to the global DeviceCache instance.
     */
    static DeviceCache &instance();

    /**
     * @brief Returns the cached hostname.
     *
     * The hostname is cached on first call to avoid repeated syscalls.
     *
     * @returns Reference to the cached hostname string.
     */
    static const QString &hostname();

    /**
     * @brief Returns a copy of all cached devices.
     *
     * This method is thread-safe and returns a copy of the device list to ensure safe access from
     * multiple threads.
     *
     * @returns List of all cached DeviceInfo objects.
     */
    QList<DeviceInfo> allDevices() const;

    /**
     * @brief Finds a device by its system path.
     * @param syspath The system path (e.g., "/sys/devices/...") to search for.
     * @returns Pointer to the DeviceInfo if found, or @c nullptr if not found.
     *
     * @note The returned pointer is valid only until the next call to refresh(). Do not store this
     *       pointer for later use.
     */
    const DeviceInfo *deviceBySyspath(const QString &syspath) const;

#ifdef Q_OS_LINUX
    /**
     * @brief Returns the udev context.
     *
     * This method is only available on Linux.
     *
     * @returns Pointer to the udev context structure.
     */
    struct udev *context() const;
#endif

    /**
     * @brief Refreshes the device cache by re-enumerating all devices.
     *
     * This method is thread-safe. After completion, the devicesChanged() signal is NOT
     * automatically emitted; it is only emitted when devices change via the udev monitor.
     *
     * @note This method blocks until enumeration is complete.
     */
    void refresh();

    /**
     * @brief Returns whether hidden devices should be shown.
     * @returns @c true if hidden devices should be displayed, @c false otherwise.
     */
    bool showHiddenDevices() const;

    /**
     * @brief Sets whether hidden devices should be shown.
     * @param show @c true to show hidden devices, @c false to hide them.
     *
     * The setting is automatically saved to persistent storage.
     */
    void setShowHiddenDevices(bool show);

    /**
     * @brief Starts monitoring for device changes.
     *
     * On Linux, this creates and starts a UdevMonitor to receive device add/remove events. When a
     * device change is detected, the cache is automatically refreshed and devicesChanged() is
     * emitted.
     *
     * On other platforms, this method does nothing.
     */
    void startMonitoring();

Q_SIGNALS:
    /**
     * @brief Emitted when devices are added or removed.
     *
     * This signal is emitted after the cache has been refreshed due to a device change detected by
     * the udev monitor.
     *
     * Connect to this signal to update UI elements when devices change.
     */
    void devicesChanged();

private Q_SLOTS:
    void onDeviceChanged();

private:
    DeviceCache();
    ~DeviceCache() override;
    DeviceCache(const DeviceCache &) = delete;
    DeviceCache &operator=(const DeviceCache &) = delete;

    void enumerate();

#ifdef Q_OS_LINUX
    UdevManager manager_;
    UdevMonitor *monitor_ = nullptr;
#elif defined(Q_OS_MACOS)
    IOKitManager manager_;
#elif defined(Q_OS_WIN)
    SetupApiManager manager_;
#endif
    QList<DeviceInfo> devices_;
    QHash<QString, int> syspathIndex_;
    mutable QMutex mutex_;
};
