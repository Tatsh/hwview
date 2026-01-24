#include "udevmanager.h"

#ifdef Q_OS_LINUX
#include "deviceinfo.h"

#include <utility>

UdevManager::UdevManager() : ctx(udev_new()) {
}

UdevManager::~UdevManager() {
    udev_unref(ctx);
}

struct udev *UdevManager::context() const {
    return ctx;
}

QList<DeviceInfo> UdevManager::convertToDeviceInfo(struct udev_enumerate *enumerator) const {
    QList<DeviceInfo> ret;
    struct udev_list_entry *listEntry;
    udev_list_entry_foreach(listEntry, udev_enumerate_get_list_entry(enumerator)) {
        ret.emplaceBack(ctx, udev_list_entry_get_name(listEntry));
    }
    std::sort(ret.begin(), ret.end(), [](auto &a, auto &b) { return a.name() < b.name(); });
    return ret;
}

QList<DeviceInfo> UdevManager::iterDevicesSubsystem(const char *subsystem) const {
    auto *enumerator = udev_enumerate_new(ctx);
    udev_enumerate_add_match_subsystem(enumerator, subsystem);
    udev_enumerate_scan_devices(enumerator);
    auto ret = convertToDeviceInfo(enumerator);
    udev_enumerate_unref(enumerator);
    return ret;
}

QList<DeviceInfo> UdevManager::iterDevicesSubsystem(const QString &subsystem) const {
    return iterDevicesSubsystem(subsystem.toLocal8Bit().constData());
}

QList<DeviceInfo> UdevManager::scanDevices(struct udev_enumerate *enumerator) const {
    udev_enumerate_scan_devices(enumerator);
    return convertToDeviceInfo(enumerator);
}

QList<DeviceInfo> UdevManager::scanDevices(const std::unique_ptr<UdevEnumerate> &wrapper) const {
    udev_enumerate_scan_devices(wrapper->enumerator());
    return convertToDeviceInfo(wrapper->enumerator());
}
#endif
