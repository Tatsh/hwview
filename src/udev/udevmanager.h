#pragma once

#include <QList>
#include <QMap>
#include <memory>

#include "deviceinfo.h"

#ifdef Q_OS_LINUX
#include "udev/udevenumerate.h"
#include <libudev.h>

class UdevManager {
public:
    UdevManager();
    ~UdevManager();

    struct udev *context() const;

    QList<DeviceInfo> iterDevicesSubsystem(const char *) const;
    QList<DeviceInfo> iterDevicesSubsystem(const QString &) const;
    QList<DeviceInfo> convertToDeviceInfo(struct udev_enumerate *) const;
    QList<DeviceInfo> scanDevices(struct udev_enumerate *) const;
    QList<DeviceInfo> scanDevices(const std::unique_ptr<UdevEnumerate> &) const;

private:
    struct udev *ctx;
};
#else
// Stub class for non-Linux platforms
class UdevManager {
public:
    UdevManager() = default;
    ~UdevManager() = default;

    void *context() const {
        return nullptr;
    }

    QList<DeviceInfo> iterDevicesSubsystem(const char *) const {
        return {};
    }
    QList<DeviceInfo> iterDevicesSubsystem(const QString &) const {
        return {};
    }
};
#endif
