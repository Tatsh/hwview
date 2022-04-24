#include <QtCore/QMap>
#include <QtCore/QVector>
#include <libudev.h>
#include <vector>

class udevManager {
public:
    udevManager();
    ~udevManager();

    QVector<QMap<QString, QString>> iterDevicesSubsystem(const char *) const;
    struct udev *context() const;

private:
    struct udev *ctx;
};
