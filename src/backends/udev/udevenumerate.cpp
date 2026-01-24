#include "udevenumerate.h"
#include "udevmanager.h"

#ifdef Q_OS_LINUX

UdevEnumerate::UdevEnumerate(UdevManager &manager)
    : enumerator_(udev_enumerate_new(manager.context())) {
}

void UdevEnumerate::addMatchProperty(const char *property) {
    udev_enumerate_add_match_property(enumerator_, property, "1");
}

void UdevEnumerate::addMatchProperty(const char *property, const QString &value) {
    udev_enumerate_add_match_property(enumerator_, property, value.toLocal8Bit().constData());
}

UdevEnumerate::~UdevEnumerate() {
    udev_enumerate_unref(enumerator_);
}

struct udev_enumerate *UdevEnumerate::enumerator() const {
    return enumerator_;
}
#endif
