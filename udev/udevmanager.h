#ifndef UDEV_H
#define UDEV_H

#include <QtCore/QMap>
#include <QtCore/QVector>
#include <libudev.h>
#include <memory>
#include <vector>

#include "deviceinfo.h"
#include "udev/udevenumerate.h"

class UdevManager {
public:
    UdevManager();
    ~UdevManager();

    struct udev *context() const;

    QVector<DeviceInfo> iterDevicesSubsystem(const char *) const;
    QVector<DeviceInfo> iterDevicesSubsystem(const QString &) const;
    QVector<DeviceInfo> convertToDeviceInfo(struct udev_enumerate *) const;
    QVector<DeviceInfo> scanDevices(struct udev_enumerate *) const;
    QVector<DeviceInfo> scanDevices(const std::unique_ptr<UdevEnumerate> &) const;

private:
    struct udev *ctx;
};

#endif // UDEV_H
