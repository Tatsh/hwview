#ifndef UDEV_H
#define UDEV_H

#include <QtCore/QMap>
#include <QtCore/QVector>
#include <libudev.h>
#include <vector>

#include "deviceinfo.h"

class udevManager {
public:
    udevManager();
    ~udevManager();

    struct udev *context() const;

    QVector<DeviceInfo> iterDevicesSubsystem(const char *) const;
    QVector<DeviceInfo>
    convertToDeviceInfo(struct udev_enumerate *enumerator) const;
    QVector<DeviceInfo> scanDevices(struct udev_enumerate *enumerator) const;

private:
    struct udev *ctx;
};

#endif // UDEV_H
