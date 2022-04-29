#ifndef UDEVENUMERATE_H
#define UDEVENUMERATE_H

#include <QtCore/QString>
#include <libudev.h>

class udevManager;

class UdevEnumerate {
public:
    UdevEnumerate(udevManager &);
    ~UdevEnumerate();
    void addMatchProperty(const char *);
    void addMatchProperty(const char *, const QString &);
    struct udev_enumerate *enumerator() const;

private:
    struct udev_enumerate *enumerator_;
};

#endif // UDEVENUMERATE_H
