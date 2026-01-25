#include "deviceinfo.h"
#include "common/importeddeviceinfo.h"
#include "deviceinfo_p.h"

#include <utility>

// Static empty objects for returning references when no data available
static const QJsonObject emptyObject;
static const QJsonArray emptyArray;
static const QString emptyString;

// DeviceInfoPrivate implementation
DeviceInfoPrivate::DeviceInfoPrivate(DeviceInfo *q) : q_ptr(q) {
    category_ = DeviceCategory::Unknown;
}

DeviceInfoPrivate::DeviceInfoPrivate(const DeviceInfoPrivate &other, DeviceInfo *q)
    : devPath_(other.devPath_), driver_(other.driver_), hidID_(other.hidID_),
      hidName_(other.hidName_), hidPhysicalMac_(other.hidPhysicalMac_), hidUniq_(other.hidUniq_),
      modAlias_(other.modAlias_), name_(other.name_), subsystem_(other.subsystem_),
      syspath_(other.syspath_), parentSyspath_(other.parentSyspath_), devnode_(other.devnode_),
      idVendorFromDatabase_(other.idVendorFromDatabase_), pciClass_(other.pciClass_),
      pciSubclass_(other.pciSubclass_), pciInterface_(other.pciInterface_),
      idCdrom_(other.idCdrom_), devType_(other.devType_), idInputKeyboard_(other.idInputKeyboard_),
      idInputMouse_(other.idInputMouse_), idType_(other.idType_),
      idModelFromDatabase_(other.idModelFromDatabase_), isHidden_(other.isHidden_),
      category_(other.category_), platformClassName_(other.platformClassName_), q_ptr(q) {
}

DeviceInfoPrivate::~DeviceInfoPrivate() = default;

// DeviceInfo implementation
DeviceInfo::DeviceInfo(DeviceInfoPrivate *d) : d_ptr(d), isImported_(false) {
    if (d_ptr) {
        d_ptr->q_ptr = this;
    }
}

DeviceInfo::DeviceInfo(const QJsonObject &json)
    : d_ptr(createDeviceInfoFromJson(json)), isImported_(true) {
    if (d_ptr) {
        d_ptr->q_ptr = this;
    }
}

DeviceInfo::~DeviceInfo() = default;

DeviceInfo::DeviceInfo(const DeviceInfo &other)
    : d_ptr(other.d_ptr ? other.d_ptr->clone(this) : nullptr), isImported_(other.isImported_) {
}

DeviceInfo &DeviceInfo::operator=(const DeviceInfo &other) {
    if (this != &other) {
        d_ptr.reset(other.d_ptr ? other.d_ptr->clone(this) : nullptr);
        isImported_ = other.isImported_;
    }
    return *this;
}

DeviceInfo::DeviceInfo(DeviceInfo &&other) noexcept
    : d_ptr(std::move(other.d_ptr)), isImported_(other.isImported_) {
    if (d_ptr) {
        d_ptr->q_ptr = this;
    }
}

DeviceInfo &DeviceInfo::operator=(DeviceInfo &&other) noexcept {
    if (this != &other) {
        d_ptr = std::move(other.d_ptr);
        if (d_ptr) {
            d_ptr->q_ptr = this;
        }
        isImported_ = other.isImported_;
    }
    return *this;
}

const QString &DeviceInfo::driver() const {
    Q_D(const DeviceInfo);
    return d ? d->driver_ : emptyString;
}

const QString &DeviceInfo::name() const {
    Q_D(const DeviceInfo);
    return d ? d->name_ : emptyString;
}

QString DeviceInfo::propertyValue(const char *key) const {
    Q_D(const DeviceInfo);
    return d ? d->propertyValue(key) : QString();
}

const QString &DeviceInfo::syspath() const {
    Q_D(const DeviceInfo);
    return d ? d->syspath_ : emptyString;
}

const QString &DeviceInfo::parentSyspath() const {
    Q_D(const DeviceInfo);
    return d ? d->parentSyspath_ : emptyString;
}

const QString &DeviceInfo::subsystem() const {
    Q_D(const DeviceInfo);
    return d ? d->subsystem_ : emptyString;
}

const QString &DeviceInfo::devnode() const {
    Q_D(const DeviceInfo);
    return d ? d->devnode_ : emptyString;
}

void DeviceInfo::dump() {
    Q_D(DeviceInfo);
    if (d) {
        d->dump();
    }
}

const QString &DeviceInfo::devPath() const {
    Q_D(const DeviceInfo);
    return d ? d->devPath_ : emptyString;
}

const QString &DeviceInfo::pciClass() const {
    Q_D(const DeviceInfo);
    return d ? d->pciClass_ : emptyString;
}

const QString &DeviceInfo::pciSubclass() const {
    Q_D(const DeviceInfo);
    return d ? d->pciSubclass_ : emptyString;
}

const QString &DeviceInfo::pciInterface() const {
    Q_D(const DeviceInfo);
    return d ? d->pciInterface_ : emptyString;
}

const QString &DeviceInfo::idCdrom() const {
    Q_D(const DeviceInfo);
    return d ? d->idCdrom_ : emptyString;
}

const QString &DeviceInfo::devType() const {
    Q_D(const DeviceInfo);
    return d ? d->devType_ : emptyString;
}

const QString &DeviceInfo::idInputKeyboard() const {
    Q_D(const DeviceInfo);
    return d ? d->idInputKeyboard_ : emptyString;
}

const QString &DeviceInfo::idInputMouse() const {
    Q_D(const DeviceInfo);
    return d ? d->idInputMouse_ : emptyString;
}

const QString &DeviceInfo::idType() const {
    Q_D(const DeviceInfo);
    return d ? d->idType_ : emptyString;
}

const QString &DeviceInfo::idModelFromDatabase() const {
    Q_D(const DeviceInfo);
    return d ? d->idModelFromDatabase_ : emptyString;
}

const QJsonObject &DeviceInfo::properties() const {
    Q_D(const DeviceInfo);
    if (d && isImported_) {
        auto *imported = dynamic_cast<const ImportedDeviceInfoPrivate *>(d);
        if (imported) {
            return imported->properties_;
        }
    }
    return emptyObject;
}

const QJsonObject &DeviceInfo::driverInfo() const {
    Q_D(const DeviceInfo);
    if (d && isImported_) {
        auto *imported = dynamic_cast<const ImportedDeviceInfoPrivate *>(d);
        if (imported) {
            return imported->driverInfo_;
        }
    }
    return emptyObject;
}

const QJsonArray &DeviceInfo::resources() const {
    Q_D(const DeviceInfo);
    if (d && isImported_) {
        auto *imported = dynamic_cast<const ImportedDeviceInfoPrivate *>(d);
        if (imported) {
            return imported->resources_;
        }
    }
    return emptyArray;
}

bool DeviceInfo::isImported() const {
    return isImported_;
}

bool DeviceInfo::isHidden() const {
    Q_D(const DeviceInfo);
    return d ? d->isHidden_ : true;
}

bool DeviceInfo::isValidForDisplay() const {
    Q_D(const DeviceInfo);
    return d && d->category_ != DeviceCategory::Unknown;
}

DeviceCategory DeviceInfo::category() const {
    Q_D(const DeviceInfo);
    return d ? d->category_ : DeviceCategory::Unknown;
}
