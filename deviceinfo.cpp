#include "deviceinfo.h"

#include "const_strings.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QVector>
#include <QtDebug>

namespace s = strings;
namespace props = strings::udev::propertyNames;

DeviceInfo::DeviceInfo(udev *ctx, const char *syspath) : ctx(ctx) {
    // udev_ref(ctx);
    dev = udev_device_new_from_syspath(ctx, syspath);
    const char *hidName = udev_device_get_property_value(dev, props::HID_NAME);
    const char *udevName = udev_device_get_property_value(dev, props::NAME);
    const char *idModelFromDatabase =
        udev_device_get_property_value(dev, props::ID_MODEL_FROM_DATABASE);
    setName();
    devPath_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::DEVPATH));
    hidID_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::HID_ID));
    hidPhysicalMac_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::HID_PHYS));
    hidUniq_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::HID_UNIQ));
    modAlias_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::MODALIAS));
    subsystem_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::SUBSYSTEM));
    driver_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::DRIVER));
    idVendorFromDatabase_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::ID_VENDOR_FROM_DATABASE));
}

const QRegularExpression quoteAtBeginning(QStringLiteral("^\""));
const QRegularExpression quoteAtEnd(QStringLiteral("\"$"));

void DeviceInfo::setName() {
    QVector<const char *> keys;
    keys << props::HID_NAME << props::NAME << props::ID_FS_LABEL
         << props::ID_PART_TABLE_UUID << props::ID_MODEL
         << props::ID_MODEL_FROM_DATABASE << props::DEVNAME << props::DM_NAME
         << props::ID_PART_ENTRY_NAME << props::ID_PCI_SUBCLASS_FROM_DATABASE
         << props::DRIVER;
    for (const char *key : keys) {
        const char *prop;
        if ((prop = udev_device_get_property_value(dev, key)) &&
            qstrlen(prop) > 0) {
            name_ = QString::fromLocal8Bit(prop)
                        .replace(quoteAtBeginning, s::empty)
                        .replace(quoteAtEnd, s::empty)
                        .replace(s::underscore, s::singleSpace)
                        .trimmed();
            break;
        }
    }
}

DeviceInfo::~DeviceInfo() {
    // udev_device_unref(dev);
    // udev_unref(ctx);
}

QString DeviceInfo::driver() const {
    return driver_;
}

QString DeviceInfo::name() const {
    return name_;
}

QString DeviceInfo::propertyValue(const char *prop) const {
    return QString::fromLocal8Bit(udev_device_get_property_value(dev, prop));
}

void DeviceInfo::dump() {
    auto firstEntry = udev_device_get_properties_list_entry(dev);
    udev_list_entry *entry;
    udev_list_entry_foreach(entry, firstEntry) {
        auto name = udev_list_entry_get_name(entry);
        qDebug() << name << udev_device_get_property_value(dev, name);
    }
}
