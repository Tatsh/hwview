#ifndef UDEV_H
#define UDEV_H

#include <QtCore/QMap>
#include <QtCore/QVector>
#include <libudev.h>
#include <vector>

class udevManager {
public:
    udevManager();
    ~udevManager();

    struct udev *context() const;

    QVector<QMap<QString, QString>> iterDevicesSubsystem(const char *) const;
    QVector<QMap<QString, QString>>
    convertToQMap(struct udev_enumerate *enumerator) const;
    QVector<QMap<QString, QString>>
    scanDevices(struct udev_enumerate *enumerator) const;

private:
    struct udev *ctx;
};

#endif // UDEV_H
