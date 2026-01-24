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

    // Start monitoring for device changes (Linux only)
    void startMonitoring();

Q_SIGNALS:
    // Emitted when devices are added or removed
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
    QHash<QString, int> syspathIndex_; // Maps syspath to index in devices_
    mutable QMutex mutex_;             // Protects devices_ and syspathIndex_
};
