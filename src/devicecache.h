#pragma once

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QMutex>
#include <QObject>

#include "deviceinfo.h"

/**
 * @brief Singleton cache that holds all device information.
 *
 * This class enumerates devices once at startup and caches them for reuse by all models. It
 * provides thread-safe access to device information and supports automatic monitoring for device
 * changes on supported platforms.
 *
 * The cache can be manually refreshed by calling @c refresh(), or it will automatically refresh
 * when devices are added or removed (on platforms with monitoring support).
 *
 * @note This is a singleton class. Use @c instance() to access the single instance.
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
     * @returns Reference to the global @c DeviceCache instance.
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
     * @returns List of all cached @c DeviceInfo objects.
     */
    QList<DeviceInfo> allDevices() const;

    /**
     * @brief Finds a device by its system path.
     * @param syspath The system path (e.g., "/sys/devices/...") to search for.
     * @returns Pointer to the @c DeviceInfo if found, or @c nullptr if not found.
     *
     * @note The returned pointer is valid only until the next call to @c refresh(). Do not store
     *       this pointer for later use.
     */
    const DeviceInfo *deviceBySyspath(const QString &syspath) const;

    /**
     * @brief Refreshes the device cache by re-enumerating all devices.
     *
     * This method is thread-safe. After completion, the @c devicesChanged() signal is NOT
     * automatically emitted; it is only emitted when devices change via the device monitor.
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
     * On supported platforms, this creates a @c DeviceMonitor to receive device add/remove events.
     * When a device change is detected, the cache is automatically refreshed and @c devicesChanged()
     * is emitted.
     *
     * On platforms without monitoring support, this method does nothing.
     */
    void startMonitoring();

    /**
     * @brief Loads device data from an export file.
     *
     * This replaces the current device cache with data loaded from the specified export file.
     * After loading, the cache enters "viewer mode" where refresh and monitoring are disabled.
     *
     * @param filePath Path to the .dmexport file.
     * @returns @c true if the file was loaded successfully, @c false otherwise.
     */
    bool loadFromFile(const QString &filePath);

    /**
     * @brief Returns whether the cache is in viewer mode.
     *
     * In viewer mode, the cache contains data loaded from an export file rather than live
     * system data. Refresh and device monitoring are disabled in this mode.
     *
     * @returns @c true if in viewer mode, @c false otherwise.
     */
    bool isViewerMode() const;

    /**
     * @brief Reloads live device data and exits viewer mode.
     *
     * This re-enumerates system devices and clears any imported data, returning to normal
     * operation mode.
     */
    void reloadLiveData();

    /**
     * @brief Returns the path of the currently loaded export file.
     * @returns The file path, or empty string if not in viewer mode.
     */
    const QString &currentFilePath() const;

    /**
     * @brief Returns the export date when in viewer mode.
     * @returns The export date string, or empty if not in viewer mode.
     */
    const QString &exportDate() const;

    /**
     * @brief Returns the source application name from the export.
     * @returns The application name, or empty if not in viewer mode.
     */
    const QString &sourceApplicationName() const;

    /**
     * @brief Returns the source application version from the export.
     * @returns The version string, or empty if not in viewer mode.
     */
    const QString &sourceApplicationVersion() const;

    /**
     * @brief Returns system information from the export.
     * @returns The system info JSON object (empty if not in viewer mode).
     */
    const QJsonObject &systemInfo() const;

    /**
     * @brief Returns system resources from the export (for Resources views).
     * @returns The system resources JSON object (empty if not in viewer mode).
     */
    const QJsonObject &systemResources() const;

Q_SIGNALS:
    /**
     * @brief Emitted when devices are added or removed.
     *
     * This signal is emitted after the cache has been refreshed due to a device change detected by
     * the device monitor.
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

    QObject *monitor_ = nullptr;
    QList<DeviceInfo> devices_;
    QHash<QString, int> syspathIndex_;
    mutable QMutex mutex_;

    // Viewer mode state
    bool viewerMode_ = false;
    QString filePath_;
    QString importedHostname_;
    QString exportDate_;
    QString sourceAppName_;
    QString sourceAppVersion_;
    QJsonObject systemInfo_;
    QJsonObject systemResources_;
};
