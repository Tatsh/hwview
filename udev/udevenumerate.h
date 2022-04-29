#ifndef UDEVENUMERATE_H
#define UDEVENUMERATE_H

#include <QtCore/QString>
#include <libudev.h>

class UdevManager;

class UdevEnumerate {
public:
    UdevEnumerate(UdevManager &);
    ~UdevEnumerate();
    void addMatchProperty(const char *);
    void addMatchProperty(const char *, const QString &);
    struct udev_enumerate *enumerator() const;

private:
    struct udev_enumerate *enumerator_;
};

#endif // UDEVENUMERATE_H
