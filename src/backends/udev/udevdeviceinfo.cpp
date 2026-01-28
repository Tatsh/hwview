// SPDX-License-Identifier: MIT
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QRegularExpression>

#include "common/const_strings_udev.h"
#include "deviceinfo.h"
#include "udevdeviceinfo_p.h"

namespace props = strings::udev::propertyNames;

static const QRegularExpression quoteAtBeginning(QStringLiteral("^\""));
static const QRegularExpression quoteAtEnd(QStringLiteral("\"$"));
static const auto underscore = QStringLiteral("_");
static const auto singleSpace = QStringLiteral(" ");
static const auto empty = QStringLiteral("");

UdevDeviceInfoPrivate::UdevDeviceInfoPrivate(udev *ctx, const char *syspath)
    : DeviceInfoPrivate(), ctx_(ctx), dev_(nullptr) {
    dev_ = udev_device_new_from_syspath(ctx, syspath);
    if (!dev_) {
        isHidden_ = true;
        return;
    }

    setName();
    devPath_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::DEVPATH));
    hidID_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::HID_ID));
    hidPhysicalMac_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::HID_PHYS));
    hidUniq_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::HID_UNIQ));
    modAlias_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::MODALIAS));
    subsystem_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::SUBSYSTEM));
    driver_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::DRIVER));
    idVendorFromDatabase_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev_, props::ID_VENDOR_FROM_DATABASE));

    pciClass_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev_, props::ID_PCI_CLASS_FROM_DATABASE));
    pciSubclass_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev_, props::ID_PCI_SUBCLASS_FROM_DATABASE));
    pciInterface_ = QString::fromLocal8Bit(
        udev_device_get_property_value(dev_, props::ID_PCI_INTERFACE_FROM_DATABASE));
    idCdrom_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::ID_CDROM));
    devType_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::DEVTYPE));
    idInputKeyboard_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::ID_INPUT_KEYBOARD));
    idInputMouse_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::ID_INPUT_MOUSE));
    idType_ = QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::ID_TYPE));
    idModelFromDatabase_ =
        QString::fromLocal8Bit(udev_device_get_property_value(dev_, props::ID_MODEL_FROM_DATABASE));
    syspath_ = QString::fromLocal8Bit(udev_device_get_syspath(dev_));

    if (auto *devnodePtr = udev_device_get_devnode(dev_)) {
        devnode_ = QString::fromLocal8Bit(devnodePtr);
    }
    if (auto *parent = udev_device_get_parent(dev_)) {
        parentSyspath_ = QString::fromLocal8Bit(udev_device_get_syspath(parent));
    }

    calculateIsHidden();
    calculateCategory();
}

UdevDeviceInfoPrivate::~UdevDeviceInfoPrivate() {
    if (dev_) {
        udev_device_unref(dev_);
    }
}

UdevDeviceInfoPrivate::UdevDeviceInfoPrivate(const UdevDeviceInfoPrivate &other, DeviceInfo *q)
    : DeviceInfoPrivate(other, q), ctx_(other.ctx_),
      dev_(other.dev_ ? udev_device_ref(other.dev_) : nullptr) {
}

void UdevDeviceInfoPrivate::setName() {
    QList<const char *> keys;
    keys << props::HID_NAME << props::NAME << props::ID_FS_LABEL << props::ID_PART_TABLE_UUID
         << props::ID_MODEL << props::ID_MODEL_FROM_DATABASE << props::DEVNAME << props::DM_NAME
         << props::ID_PART_ENTRY_NAME << props::ID_PCI_SUBCLASS_FROM_DATABASE << props::DRIVER;
    for (const auto *key : keys) {
        if (const auto *prop = udev_device_get_property_value(dev_, key);
            prop && qstrlen(prop) > 0) {
            name_ = QString::fromLocal8Bit(prop)
                        .replace(quoteAtBeginning, empty)
                        .replace(quoteAtEnd, empty)
                        .replace(underscore, singleSpace)
                        .trimmed();
            break;
        }
    }
}

void UdevDeviceInfoPrivate::calculateIsHidden() {
    if (devPath_.startsWith(QStringLiteral("/devices/virtual/"))) {
        isHidden_ = true;
        return;
    }

    if (driver_.isEmpty() && name_.isEmpty()) {
        isHidden_ = true;
        return;
    }

    isHidden_ = false;
}

void UdevDeviceInfoPrivate::calculateCategory() {
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
        if (idCdrom_ == QStringLiteral("1")) {
            category_ = DeviceCategory::DvdCdromDrives;
            return;
        }
        if (devType_ == us::propertyValues::devType::partition()) {
            category_ = DeviceCategory::StorageVolumes;
            return;
        }
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

    // Software devices
    if (subsystem_ == us::subsystems::misc()) {
        category_ = DeviceCategory::SoftwareDevices;
        return;
    }

    // System devices
    if (subsystem_ == us::subsystems::pci()) {
        category_ = DeviceCategory::SystemDevices;
        return;
    }

    category_ = DeviceCategory::Unknown;
}

QString UdevDeviceInfoPrivate::propertyValue(const char *key) const {
    if (!dev_) {
        return {};
    }
    return QString::fromLocal8Bit(udev_device_get_property_value(dev_, key));
}

DeviceInfoPrivate *UdevDeviceInfoPrivate::clone(DeviceInfo *q) const {
    return new UdevDeviceInfoPrivate(*this, q);
}

void UdevDeviceInfoPrivate::dump() const {
    if (!dev_) {
        qDebug() << "UdevDeviceInfoPrivate: dev is null";
        return;
    }
    auto *firstEntry = udev_device_get_properties_list_entry(dev_);
    udev_list_entry *entry;
    udev_list_entry_foreach(entry, firstEntry) {
        auto *name = udev_list_entry_get_name(entry);
        qDebug() << name << udev_device_get_property_value(dev_, name);
    }
}

DeviceInfoPrivate *createDeviceInfo(udev *ctx, const char *syspath) {
    return new UdevDeviceInfoPrivate(ctx, syspath);
}
