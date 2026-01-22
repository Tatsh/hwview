#include "deviceinfo.h"

#include "const_strings.h"

#include <QDebug>
#include <QList>
#include <QRegularExpression>
#include <utility>

namespace s = strings;
namespace props = strings::udev::propertyNames;

#ifdef Q_OS_LINUX
DeviceInfo::DeviceInfo(udev *ctx_, const char *syspathArg) : ctx(ctx_), dev(nullptr) {
    dev = udev_device_new_from_syspath(ctx, syspathArg);
    if (!dev) {
        // Device creation failed - mark as hidden and return with empty data
        isHidden_ = true;
        return;
    }
    setName();
    devPath_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::DEVPATH));
    hidID_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::HID_ID));
    hidPhysicalMac_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::HID_PHYS));
    hidUniq_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::HID_UNIQ));
    modAlias_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::MODALIAS));
    subsystem_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::SUBSYSTEM));
    driver_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::DRIVER));
    idVendorFromDatabase_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev, props::ID_VENDOR_FROM_DATABASE));
    // Cache frequently-accessed properties
    pciClass_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::ID_PCI_CLASS_FROM_DATABASE));
    pciSubclass_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::ID_PCI_SUBCLASS_FROM_DATABASE));
    pciInterface_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev, props::ID_PCI_INTERFACE_FROM_DATABASE));
    idCdrom_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::ID_CDROM));
    devType_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::DEVTYPE));
    idInputKeyboard_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev, props::ID_INPUT_KEYBOARD));
    idInputMouse_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev, props::ID_INPUT_MOUSE));
    idType_ = QString::fromLocal8Bit(udev_device_get_property_value(dev, props::ID_TYPE));
    idModelFromDatabase_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev, props::ID_MODEL_FROM_DATABASE));
    syspath_ = QString::fromLocal8Bit(udev_device_get_syspath(dev));
    if (auto *devnodePtr = udev_device_get_devnode(dev)) {
        devnode_ = QString::fromLocal8Bit(devnodePtr);
    }
    if (auto *parent = udev_device_get_parent(dev)) {
        parentSyspath_ = QString::fromLocal8Bit(udev_device_get_syspath(parent));
    }
    calculateIsHidden();
    calculateCategory();
}

static const QRegularExpression quoteAtBeginning(QStringLiteral("^\""));
static const QRegularExpression quoteAtEnd(QStringLiteral("\"$"));

void DeviceInfo::setName() {
    QList<const char *> keys;
    keys << props::HID_NAME << props::NAME << props::ID_FS_LABEL << props::ID_PART_TABLE_UUID
         << props::ID_MODEL << props::ID_MODEL_FROM_DATABASE << props::DEVNAME << props::DM_NAME
         << props::ID_PART_ENTRY_NAME << props::ID_PCI_SUBCLASS_FROM_DATABASE << props::DRIVER;
    for (const auto *key : keys) {
        if (const auto *prop = udev_device_get_property_value(dev, key);
            prop && qstrlen(prop) > 0) {
            name_ = QString::fromLocal8Bit(prop)
                        .replace(quoteAtBeginning, s::empty())
                        .replace(quoteAtEnd, s::empty())
                        .replace(s::underscore(), s::singleSpace())
                        .trimmed();
            break;
        }
    }
}

void DeviceInfo::calculateIsHidden() {
    // Hidden devices are:
    // 1. Devices in /sys/devices/virtual/ paths (Linux-specific)
    // 2. Devices without a driver
    // 3. Devices without a name
#ifdef Q_OS_LINUX
    if (devPath_.startsWith(QStringLiteral("/devices/virtual/"))) {
        isHidden_ = true;
        return;
    }
#endif
    if (driver_.isEmpty()) {
        isHidden_ = true;
        return;
    }
    if (name_.isEmpty()) {
        isHidden_ = true;
        return;
    }
    isHidden_ = false;
}

void DeviceInfo::calculateCategory() {
    namespace us = strings::udev;

    // Audio inputs and outputs
    if (pciSubclass_ == us::propertyValues::idPciSubclassFromDatabase::audioDevice()) {
        category_ = DeviceCategory::AudioInputsAndOutputs;
        return;
    }

    // Batteries
    if (idModelFromDatabase_ == us::propertyValues::idModelFromDatabase::ups() ||
        driver_ == us::propertyValues::driver::battery()) {
        category_ = DeviceCategory::Batteries;
        return;
    }

    // Display adapters
    if (pciClass_ == us::propertyValues::idPciClassFromDatabase::displayController()) {
        category_ = DeviceCategory::DisplayAdapters;
        return;
    }

    // USB controllers
    if (pciSubclass_ == us::propertyValues::idPciSubclassFromDatabase::usbController()) {
        category_ = DeviceCategory::UniversalSerialBusControllers;
        return;
    }

    // Storage controllers
    if (pciClass_ == us::propertyValues::idPciClassFromDatabase::massStorageController()) {
        category_ = DeviceCategory::StorageControllers;
        return;
    }

    // Network adapters
    if (pciClass_ == us::propertyValues::idPciClassFromDatabase::networkController()) {
        category_ = DeviceCategory::NetworkAdapters;
        return;
    }

    // Block devices
    if (subsystem_ == us::subsystems::block()) {
        // CD-ROMs
        if (idCdrom_ == QStringLiteral("1")) {
            category_ = DeviceCategory::DvdCdromDrives;
            return;
        }
        // Partitions (storage volumes)
        if (devType_ == us::propertyValues::devType::partition()) {
            category_ = DeviceCategory::StorageVolumes;
            return;
        }
        // Disk drives (non-virtual)
        if (!devPath_.startsWith(QStringLiteral("/devices/virtual/"))) {
            category_ = DeviceCategory::DiskDrives;
            return;
        }
    }

    // HID devices
    if (subsystem_ == us::subsystems::hid()) {
        category_ = DeviceCategory::HumanInterfaceDevices;
        return;
    }

    // Keyboards
    if (!idInputKeyboard_.isEmpty()) {
        category_ = DeviceCategory::Keyboards;
        return;
    }

    // Mice
    if (!idInputMouse_.isEmpty()) {
        category_ = DeviceCategory::MiceAndOtherPointingDevices;
        return;
    }

    // Sound/video/game controllers
    if (idType_ == us::propertyValues::idType::audio()) {
        category_ = DeviceCategory::SoundVideoAndGameControllers;
        return;
    }

    // Software devices (misc subsystem)
    if (subsystem_ == us::subsystems::misc()) {
        category_ = DeviceCategory::SoftwareDevices;
        return;
    }

    // System devices (PCI devices not categorized elsewhere)
    if (subsystem_ == us::subsystems::pci()) {
        category_ = DeviceCategory::SystemDevices;
        return;
    }

    category_ = DeviceCategory::Unknown;
}

DeviceCategory DeviceInfo::category() const {
    return category_;
}

QString DeviceInfo::propertyValue(const char *prop) const {
    if (!dev) {
        return {};
    }
    return QString::fromLocal8Bit(udev_device_get_property_value(dev, prop));
}

void DeviceInfo::dump() {
    if (!dev) {
        qDebug() << "DeviceInfo: dev is null";
        return;
    }
    auto *firstEntry = udev_device_get_properties_list_entry(dev);
    udev_list_entry *entry;
    udev_list_entry_foreach(entry, firstEntry) {
        auto *name = udev_list_entry_get_name(entry);
        qDebug() << name << udev_device_get_property_value(dev, name);
    }
}
#else
QString DeviceInfo::propertyValue([[maybe_unused]] const char *prop) const {
    return QString();
}

void DeviceInfo::dump() {
}
#endif

DeviceInfo::~DeviceInfo() {
#ifdef Q_OS_LINUX
    if (dev) {
        udev_device_unref(dev);
    }
#endif
}

DeviceInfo::DeviceInfo(const DeviceInfo &other)
    : devPath_(other.devPath_), driver_(other.driver_), hidID_(other.hidID_),
      hidName_(other.hidName_), hidPhysicalMac_(other.hidPhysicalMac_), hidUniq_(other.hidUniq_),
      modAlias_(other.modAlias_), name_(other.name_), subsystem_(other.subsystem_),
      syspath_(other.syspath_), parentSyspath_(other.parentSyspath_), devnode_(other.devnode_),
      idVendorFromDatabase_(other.idVendorFromDatabase_), pciClass_(other.pciClass_),
      pciSubclass_(other.pciSubclass_), pciInterface_(other.pciInterface_),
      idCdrom_(other.idCdrom_), devType_(other.devType_), idInputKeyboard_(other.idInputKeyboard_),
      idInputMouse_(other.idInputMouse_), idType_(other.idType_),
      idModelFromDatabase_(other.idModelFromDatabase_), isHidden_(other.isHidden_),
      category_(other.category_) {
#ifdef Q_OS_LINUX
    ctx = other.ctx;
    dev = other.dev ? udev_device_ref(other.dev) : nullptr;
#endif
}

DeviceInfo &DeviceInfo::operator=(const DeviceInfo &other) {
    if (this != &other) {
#ifdef Q_OS_LINUX
        if (dev) {
            udev_device_unref(dev);
        }
        ctx = other.ctx;
        dev = other.dev ? udev_device_ref(other.dev) : nullptr;
#endif
        devPath_ = other.devPath_;
        driver_ = other.driver_;
        hidID_ = other.hidID_;
        hidName_ = other.hidName_;
        hidPhysicalMac_ = other.hidPhysicalMac_;
        hidUniq_ = other.hidUniq_;
        modAlias_ = other.modAlias_;
        name_ = other.name_;
        subsystem_ = other.subsystem_;
        syspath_ = other.syspath_;
        parentSyspath_ = other.parentSyspath_;
        devnode_ = other.devnode_;
        idVendorFromDatabase_ = other.idVendorFromDatabase_;
        pciClass_ = other.pciClass_;
        pciSubclass_ = other.pciSubclass_;
        pciInterface_ = other.pciInterface_;
        idCdrom_ = other.idCdrom_;
        devType_ = other.devType_;
        idInputKeyboard_ = other.idInputKeyboard_;
        idInputMouse_ = other.idInputMouse_;
        idType_ = other.idType_;
        idModelFromDatabase_ = other.idModelFromDatabase_;
        isHidden_ = other.isHidden_;
        category_ = other.category_;
    }
    return *this;
}

DeviceInfo::DeviceInfo(DeviceInfo &&other) noexcept
    : devPath_(std::move(other.devPath_)), driver_(std::move(other.driver_)),
      hidID_(std::move(other.hidID_)), hidName_(std::move(other.hidName_)),
      hidPhysicalMac_(std::move(other.hidPhysicalMac_)), hidUniq_(std::move(other.hidUniq_)),
      modAlias_(std::move(other.modAlias_)), name_(std::move(other.name_)),
      subsystem_(std::move(other.subsystem_)), syspath_(std::move(other.syspath_)),
      parentSyspath_(std::move(other.parentSyspath_)), devnode_(std::move(other.devnode_)),
      idVendorFromDatabase_(std::move(other.idVendorFromDatabase_)),
      pciClass_(std::move(other.pciClass_)), pciSubclass_(std::move(other.pciSubclass_)),
      pciInterface_(std::move(other.pciInterface_)), idCdrom_(std::move(other.idCdrom_)),
      devType_(std::move(other.devType_)), idInputKeyboard_(std::move(other.idInputKeyboard_)),
      idInputMouse_(std::move(other.idInputMouse_)), idType_(std::move(other.idType_)),
      idModelFromDatabase_(std::move(other.idModelFromDatabase_)), isHidden_(other.isHidden_),
      category_(other.category_) {
#ifdef Q_OS_LINUX
    ctx = other.ctx;
    dev = other.dev;
    other.dev = nullptr; // Prevent double-unref
#endif
}

DeviceInfo &DeviceInfo::operator=(DeviceInfo &&other) noexcept {
    if (this != &other) {
#ifdef Q_OS_LINUX
        if (dev) {
            udev_device_unref(dev);
        }
        ctx = other.ctx;
        dev = other.dev;
        other.dev = nullptr;
#endif
        devPath_ = std::move(other.devPath_);
        driver_ = std::move(other.driver_);
        hidID_ = std::move(other.hidID_);
        hidName_ = std::move(other.hidName_);
        hidPhysicalMac_ = std::move(other.hidPhysicalMac_);
        hidUniq_ = std::move(other.hidUniq_);
        modAlias_ = std::move(other.modAlias_);
        name_ = std::move(other.name_);
        subsystem_ = std::move(other.subsystem_);
        syspath_ = std::move(other.syspath_);
        parentSyspath_ = std::move(other.parentSyspath_);
        devnode_ = std::move(other.devnode_);
        idVendorFromDatabase_ = std::move(other.idVendorFromDatabase_);
        pciClass_ = std::move(other.pciClass_);
        pciSubclass_ = std::move(other.pciSubclass_);
        pciInterface_ = std::move(other.pciInterface_);
        idCdrom_ = std::move(other.idCdrom_);
        devType_ = std::move(other.devType_);
        idInputKeyboard_ = std::move(other.idInputKeyboard_);
        idInputMouse_ = std::move(other.idInputMouse_);
        idType_ = std::move(other.idType_);
        idModelFromDatabase_ = std::move(other.idModelFromDatabase_);
        isHidden_ = other.isHidden_;
        category_ = other.category_;
    }
    return *this;
}

const QString &DeviceInfo::driver() const {
    return driver_;
}

const QString &DeviceInfo::name() const {
    return name_;
}

const QString &DeviceInfo::syspath() const {
    return syspath_;
}

const QString &DeviceInfo::parentSyspath() const {
    return parentSyspath_;
}

const QString &DeviceInfo::subsystem() const {
    return subsystem_;
}

const QString &DeviceInfo::devnode() const {
    return devnode_;
}

const QString &DeviceInfo::devPath() const {
    return devPath_;
}

const QString &DeviceInfo::pciClass() const {
    return pciClass_;
}

const QString &DeviceInfo::pciSubclass() const {
    return pciSubclass_;
}

const QString &DeviceInfo::pciInterface() const {
    return pciInterface_;
}

const QString &DeviceInfo::idCdrom() const {
    return idCdrom_;
}

const QString &DeviceInfo::devType() const {
    return devType_;
}

const QString &DeviceInfo::idInputKeyboard() const {
    return idInputKeyboard_;
}

const QString &DeviceInfo::idInputMouse() const {
    return idInputMouse_;
}

const QString &DeviceInfo::idType() const {
    return idType_;
}

const QString &DeviceInfo::idModelFromDatabase() const {
    return idModelFromDatabase_;
}

bool DeviceInfo::isHidden() const {
    return isHidden_;
}

bool DeviceInfo::isValidForDisplay() const {
    namespace us = strings::udev;

    // Audio devices
    if (pciSubclass_ == us::propertyValues::idPciSubclassFromDatabase::audioDevice()) {
        return true;
    }

    // Batteries
    if (idModelFromDatabase_ == us::propertyValues::idModelFromDatabase::ups() ||
        driver_ == us::propertyValues::driver::battery()) {
        return true;
    }

    // Block devices
    if (subsystem_ == us::subsystems::block()) {
        return true;
    }

    // Display adapters
    if (pciClass_ == us::propertyValues::idPciClassFromDatabase::displayController()) {
        return true;
    }

    // HID devices
    if (subsystem_ == us::subsystems::hid()) {
        return true;
    }

    // Keyboards
    if (!idInputKeyboard_.isEmpty()) {
        return true;
    }

    // Mice
    if (!idInputMouse_.isEmpty()) {
        return true;
    }

    // Network adapters
    if (pciClass_ == us::propertyValues::idPciClassFromDatabase::networkController()) {
        return true;
    }

    // Misc/software devices
    if (subsystem_ == us::subsystems::misc()) {
        return true;
    }

    // Sound/video/game controllers
    if (idType_ == us::propertyValues::idType::audio()) {
        return true;
    }

    // Storage controllers
    if (pciClass_ == us::propertyValues::idPciClassFromDatabase::massStorageController()) {
        return true;
    }

    // System devices (PCI)
    if (subsystem_ == us::subsystems::pci()) {
        return true;
    }

    // USB controllers
    if (pciSubclass_ == us::propertyValues::idPciSubclassFromDatabase::usbController()) {
        return true;
    }

    return false;
}
