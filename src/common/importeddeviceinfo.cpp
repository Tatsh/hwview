#include "importeddeviceinfo.h"
#include "deviceinfo.h"

#include <QDebug>

ImportedDeviceInfoPrivate::ImportedDeviceInfoPrivate(const QJsonObject &json)
    : DeviceInfoPrivate() {
    syspath_ = json[QStringLiteral("syspath")].toString();
    name_ = json[QStringLiteral("name")].toString();
    driver_ = json[QStringLiteral("driver")].toString();
    subsystem_ = json[QStringLiteral("subsystem")].toString();
    devnode_ = json[QStringLiteral("devnode")].toString();
    parentSyspath_ = json[QStringLiteral("parentSyspath")].toString();
    devPath_ = json[QStringLiteral("devPath")].toString();

    // PCI info
    pciClass_ = json[QStringLiteral("pciClass")].toString();
    pciSubclass_ = json[QStringLiteral("pciSubclass")].toString();
    pciInterface_ = json[QStringLiteral("pciInterface")].toString();

    // ID properties
    idCdrom_ = json[QStringLiteral("idCdrom")].toString();
    devType_ = json[QStringLiteral("idDevType")].toString();
    idInputKeyboard_ = json[QStringLiteral("idInputKeyboard")].toString();
    idInputMouse_ = json[QStringLiteral("idInputMouse")].toString();
    idType_ = json[QStringLiteral("idType")].toString();
    idModelFromDatabase_ = json[QStringLiteral("idModelFromDatabase")].toString();

    // Hidden and category are pre-computed in the export
    isHidden_ = json[QStringLiteral("isHidden")].toBool();
    category_ = static_cast<DeviceCategory>(json[QStringLiteral("category")].toInt());

    // Store imported-only data
    properties_ = json[QStringLiteral("properties")].toObject();
    driverInfo_ = json[QStringLiteral("driverInfo")].toObject();
    resources_ = json[QStringLiteral("resources")].toArray();
}

ImportedDeviceInfoPrivate::ImportedDeviceInfoPrivate(const ImportedDeviceInfoPrivate &other,
                                                     DeviceInfo *q)
    : DeviceInfoPrivate(other, q), properties_(other.properties_), driverInfo_(other.driverInfo_),
      resources_(other.resources_) {
}

QString ImportedDeviceInfoPrivate::propertyValue(const char *key) const {
    return properties_[QString::fromLatin1(key)].toString();
}

DeviceInfoPrivate *ImportedDeviceInfoPrivate::clone(DeviceInfo *q) const {
    return new ImportedDeviceInfoPrivate(*this, q);
}

void ImportedDeviceInfoPrivate::dump() const {
    qDebug() << "ImportedDeviceInfoPrivate:";
    qDebug() << "  syspath:" << syspath_;
    qDebug() << "  name:" << name_;
    qDebug() << "  driver:" << driver_;
    qDebug() << "  category:" << static_cast<int>(category_);
    qDebug() << "  isHidden:" << isHidden_;
}

DeviceInfoPrivate *createDeviceInfoFromJson(const QJsonObject &json) {
    return new ImportedDeviceInfoPrivate(json);
}
