#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QtCore/QString>
#include <libudev.h>

class DeviceInfo {
public:
    DeviceInfo(udev *, const char *);
    ~DeviceInfo();

    QString &driver();
    QString &name();
    QString propertyValue(const char *);

private:
    void setName();

    udev *ctx;
    udev_device *dev;
    QString devPath_;
    QString driver_;
    QString hidID_;
    QString hidName_;
    QString hidPhysicalMAC_;
    QString hidUniq_;
    QString modAlias_;
    QString name_;
    QString subsystem_;
    QString idVendorFromDatabase_;
};

#endif // DEVICEINFO_H
