#pragma once

#include <QHash>
#include <QList>
#include <QMutex>
#include <QObject>

#include "deviceinfo.h"
#include "udev/udevmanager.h"

#ifdef Q_OS_LINUX
// Forward declaration for udev context
struct udev;
#endif

// Singleton cache that holds all device information.
// Devices are enumerated once and reused by all models.
class DeviceCache : public QObject {
    Q_OBJECT

public:
    static DeviceCache &instance();

    // Get cached hostname (avoids repeated syscalls)
    static const QString &hostname();

    // Get all cached devices (returns a copy for thread safety)
    QList<DeviceInfo> allDevices() const;

    // Get device by syspath
    const DeviceInfo *deviceBySyspath(const QString &syspath) const;

#ifdef Q_OS_LINUX
    // Get the udev context (Linux only)
    struct udev *context() const;
#endif

    // Refresh the cache (re-enumerate all devices) - thread-safe
    void refresh();

    // Show hidden devices setting
    bool showHiddenDevices() const;
    void setShowHiddenDevices(bool show);

private:
    DeviceCache();
    ~DeviceCache() override;
    DeviceCache(const DeviceCache &) = delete;
    DeviceCache &operator=(const DeviceCache &) = delete;

    void enumerate();

    UdevManager manager_;
    QList<DeviceInfo> devices_;
    QHash<QString, int> syspathIndex_; // Maps syspath to index in devices_
    mutable QMutex mutex_;             // Protects devices_ and syspathIndex_
};
