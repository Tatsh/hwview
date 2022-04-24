#include <QtDebug>

#include "udev.h"

udevManager::udevManager() : ctx(udev_new()) {
}

udevManager::~udevManager() {
    udev_unref(ctx);
}

struct udev *udevManager::context() const {
    return ctx;
}

QVector<QMap<QString, QString>>
udevManager::iterDevicesSubsystem(const char *subsystem) const {
    auto enumerator = udev_enumerate_new(ctx);
    udev_enumerate_add_match_subsystem(enumerator, subsystem);
    udev_enumerate_scan_devices(enumerator);
    QVector<QMap<QString, QString>> ret;
    struct udev_list_entry *listEntry, *deviceProperty;
    udev_list_entry_foreach(listEntry,
                            udev_enumerate_get_list_entry(enumerator)) {
        auto device = udev_device_new_from_syspath(
            ctx, udev_list_entry_get_name(listEntry));
        QMap<QString, QString> entry;
        udev_list_entry_foreach(
            deviceProperty, udev_device_get_properties_list_entry(device)) {
            auto key = udev_list_entry_get_name(deviceProperty);
            entry.insert(QString::fromLocal8Bit(key),
                         QString::fromLocal8Bit(
                             udev_device_get_property_value(device, key)));
        }
        ret.append(entry);
        udev_device_unref(device);
    }
    udev_enumerate_unref(enumerator);
    return ret;
}
