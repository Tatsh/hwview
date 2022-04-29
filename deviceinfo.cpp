#include "deviceinfo.h"

#include "const_strings.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QVector>

DeviceInfo::DeviceInfo(udev *ctx, const char *syspath) : ctx(ctx) {
    // udev_ref(ctx);
    dev = udev_device_new_from_syspath(ctx, syspath);
    const char *hidName = udev_device_get_property_value(dev, "HID_NAME");
    const char *udevName = udev_device_get_property_value(dev, "NAME");
    const char *idModelFromDatabase =
        udev_device_get_property_value(dev, "ID_MODEL_FROM_DATABASE");
    setName();
    devPath_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev, "DEVPATH"));
    hidID_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev, "HID_ID"));
    hidPhysicalMAC_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, "HID_PHYS"));
    hidUniq_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, "HID_UNIQ"));
    modAlias_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, "MODALIAS"));
    subsystem_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, "SUBSYSTEM"));
    driver_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev, "DRIVER"));
    idVendorFromDatabase_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, "ID_VENDOR_FROM_DATABASE"));
}

void DeviceInfo::setName() {
    QVector<const char *> keys;
    keys << "HID_NAME"
         << "NAME"
         << "ID_FS_LABEL"
         << "ID_PART_TABLE_UUID"
         << "ID_MODEL"
         << "ID_MODEL_FROM_DATABASE"
         << "DEVNAME"
         << "DM_NAME"
         << "ID_PART_ENTRY_NAME";
    for (const char *key : keys) {
        const char *prop;
        if ((prop = udev_device_get_property_value(dev, key))) {
            name_ = QString::fromLocal8Bit(prop)
                        .replace(QRegularExpression(QStringLiteral("^\"")),
                                 strings::empty)
                        .replace(QRegularExpression(QStringLiteral("\"$")),
                                 strings::empty)
                        .replace(QStringLiteral("_"), strings::singleSpace)
                        .trimmed();
            break;
        }
    }
}

DeviceInfo::~DeviceInfo() {
    // udev_device_unref(dev);
    // udev_unref(ctx);
}

QString &DeviceInfo::driver() {
    return driver_;
}

QString &DeviceInfo::name() {
    return name_;
}

QString DeviceInfo::propertyValue(const char *prop) {
    return QString::fromLocal8Bit(udev_device_get_property_value(dev, prop));
}
