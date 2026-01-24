#include "devicecache.h"
#include "devicemonitor.h"
#include "viewsettings.h"

#include <QHostInfo>
#include <QMutexLocker>

#ifdef Q_OS_LINUX
#include "udev/udevmonitor.h"
#include <libudev.h>
#endif

DeviceCache &DeviceCache::instance() {
    static DeviceCache cache;
    return cache;
}

const QString &DeviceCache::hostname() {
    static QString cachedHostname = QHostInfo::localHostName();
    return cachedHostname;
}

DeviceCache::DeviceCache() : QObject(nullptr) {
    enumerate();
    startMonitoring();
}

DeviceCache::~DeviceCache() {
    if (monitor_) {
        monitor_->stop();
        delete monitor_;
        monitor_ = nullptr;
    }
}

void DeviceCache::enumerate() {
    devices_.clear();
    syspathIndex_.clear();

#ifdef Q_OS_LINUX
    auto *enumerator = udev_enumerate_new(manager_.context());
    udev_enumerate_scan_devices(enumerator);
    struct udev_list_entry *listEntry;
    udev_list_entry_foreach(listEntry, udev_enumerate_get_list_entry(enumerator)) {
        auto *syspath = udev_list_entry_get_name(listEntry);
        devices_.emplaceBack(manager_.context(), syspath);
        syspathIndex_.insert(devices_.last().syspath(), devices_.size() - 1);
    }
    udev_enumerate_unref(enumerator);
#elif defined(Q_OS_MACOS)
    manager_.enumerateAllDevices([this](io_service_t service) {
        devices_.emplaceBack(service);
        const QString &syspath = devices_.last().syspath();
        if (!syspath.isEmpty()) {
            syspathIndex_.insert(syspath, devices_.size() - 1);
        }
    });
#elif defined(Q_OS_WIN)
    manager_.enumerateAllDevices([this](HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
        devices_.emplaceBack(devInfo, devInfoData);
        const QString &syspath = devices_.last().syspath();
        if (!syspath.isEmpty()) {
            syspathIndex_.insert(syspath, devices_.size() - 1);
        }
    });
#endif
}

QList<DeviceInfo> DeviceCache::allDevices() const {
    QMutexLocker locker(&mutex_);
    return devices_; // Returns a copy for thread safety
}

const DeviceInfo *DeviceCache::deviceBySyspath(const QString &syspath) const {
    QMutexLocker locker(&mutex_);
    auto it = syspathIndex_.find(syspath);
    if (it != syspathIndex_.end()) {
        return &devices_.at(it.value());
    }
    return nullptr;
}

#ifdef Q_OS_LINUX
struct udev *DeviceCache::context() const {
    return manager_.context();
}
#endif

void DeviceCache::refresh() {
    QMutexLocker locker(&mutex_);
    enumerate();
}

bool DeviceCache::showHiddenDevices() const {
    return ViewSettings::instance().showHiddenDevices();
}

void DeviceCache::setShowHiddenDevices(bool show) {
    ViewSettings::instance().setShowHiddenDevices(show);
    ViewSettings::instance().save();
}

void DeviceCache::startMonitoring() {
#ifdef Q_OS_LINUX
    monitor_ = new UdevMonitor(manager_.context(), this);
    connect(monitor_, &DeviceMonitor::deviceChanged, this, &DeviceCache::onDeviceChanged);
    monitor_->start();
#endif
    // Other platforms don't have monitoring support yet
}

void DeviceCache::onDeviceChanged() {
    refresh();
    Q_EMIT devicesChanged();
}
