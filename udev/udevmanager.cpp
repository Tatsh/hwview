#include "udev/udevmanager.h"
#include "deviceinfo.h"

udevManager::udevManager() : ctx(udev_new()) {
}

udevManager::~udevManager() {
    udev_unref(ctx);
}

struct udev *udevManager::context() const {
    return ctx;
}

QVector<DeviceInfo>
udevManager::convertToDeviceInfo(struct udev_enumerate *enumerator) const {
    QVector<DeviceInfo> ret;
    struct udev_list_entry *listEntry, *deviceProperty;
    udev_list_entry_foreach(listEntry,
                            udev_enumerate_get_list_entry(enumerator)) {
        auto info = DeviceInfo(ctx, udev_list_entry_get_name(listEntry));
        ret.append(info);
    }
    std::sort(ret.begin(), ret.end(), [](DeviceInfo &a, DeviceInfo &b) {
        return a.name() < b.name();
    });
    return ret;
}

QVector<DeviceInfo>
udevManager::iterDevicesSubsystem(const char *subsystem) const {
    auto enumerator = udev_enumerate_new(ctx);
    udev_enumerate_add_match_subsystem(enumerator, subsystem);
    udev_enumerate_scan_devices(enumerator);
    auto ret = convertToDeviceInfo(enumerator);
    udev_enumerate_unref(enumerator);
    return ret;
}

QVector<DeviceInfo>
udevManager::iterDevicesSubsystem(const QString &subsystem) const {
    return iterDevicesSubsystem(subsystem.toLocal8Bit().constData());
}

QVector<DeviceInfo>
udevManager::scanDevices(struct udev_enumerate *enumerator) const {
    udev_enumerate_scan_devices(enumerator);
    return convertToDeviceInfo(enumerator);
}

QVector<DeviceInfo>
udevManager::scanDevices(const std::unique_ptr<UdevEnumerate> &wrapper) const {
    udev_enumerate_scan_devices(wrapper->enumerator());
    return convertToDeviceInfo(wrapper->enumerator());
}
