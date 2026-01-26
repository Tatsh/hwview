#pragma once

#include <QtCore/QString>

namespace strings {

/**
 * @brief Namespace containing udev-related constants and string accessors.
 */
namespace udev {

/**
 * @brief udev property name constants.
 */
namespace propertyNames {
constexpr char DEVNAME[] = "DEVNAME";                               ///< Device node name.
constexpr char DEVPATH[] = "DEVPATH";                               ///< Device path in sysfs.
constexpr char DEVTYPE[] = "DEVTYPE";                               ///< Device type.
constexpr char DM_NAME[] = "DM_NAME";                               ///< Device mapper name.
constexpr char DRIVER[] = "DRIVER";                                 ///< Driver name.
constexpr char HID_NAME[] = "HID_NAME";                             ///< HID device name.
constexpr char HID_ID[] = "HID_ID";                                 ///< HID device ID.
constexpr char HID_PHYS[] = "HID_PHYS";                             ///< HID physical path.
constexpr char HID_UNIQ[] = "HID_UNIQ";                             ///< HID unique ID.
constexpr char ID_CDROM[] = "ID_CDROM";                             ///< CD-ROM indicator.
constexpr char ID_FS_LABEL[] = "ID_FS_LABEL";                       ///< Filesystem label.
constexpr char ID_PART_ENTRY_NAME[] = "ID_PART_ENTRY_NAME";         ///< Partition name.
constexpr char ID_PART_TABLE_UUID[] = "ID_PART_TABLE_UUID";         ///< Partition table UUID.
constexpr char ID_INPUT_KEYBOARD[] = "ID_INPUT_KEYBOARD";           ///< Keyboard indicator.
constexpr char ID_INPUT_MOUSE[] = "ID_INPUT_MOUSE";                 ///< Mouse indicator.
constexpr char ID_MODEL[] = "ID_MODEL";                             ///< Device model.
constexpr char ID_MODEL_FROM_DATABASE[] = "ID_MODEL_FROM_DATABASE"; ///< Model from hwdb.
constexpr char ID_PCI_CLASS_FROM_DATABASE[] =
    "ID_PCI_CLASS_FROM_DATABASE"; ///< PCI class from hwdb.
constexpr char ID_PCI_INTERFACE_FROM_DATABASE[] =
    "ID_PCI_INTERFACE_FROM_DATABASE"; ///< PCI interface.
constexpr char ID_PCI_SUBCLASS_FROM_DATABASE[] = "ID_PCI_SUBCLASS_FROM_DATABASE"; ///< PCI subclass.
constexpr char ID_TYPE[] = "ID_TYPE";                                             ///< ID type.
constexpr char ID_USB_VENDOR[] = "ID_USB_VENDOR";                     ///< USB vendor name.
constexpr char ID_VENDOR[] = "ID_VENDOR";                             ///< Vendor name.
constexpr char ID_VENDOR_ENC[] = "ID_VENDOR_ENC";                     ///< Encoded vendor name.
constexpr char ID_VENDOR_FROM_DATABASE[] = "ID_VENDOR_FROM_DATABASE"; ///< Vendor from hwdb.
constexpr char MODALIAS[] = "MODALIAS";                               ///< Module alias.
constexpr char NAME[] = "NAME";                                       ///< Device name.
constexpr char SUBSYSTEM[] = "SUBSYSTEM";                             ///< Subsystem name.
} // namespace propertyNames

/**
 * @brief udev property value constants organized by property type.
 */
namespace propertyValues {

/**
 * @brief DEVTYPE property values.
 */
namespace devType {
inline const QString &disk() {
    static const auto s = QStringLiteral("disk");
    return s;
}
inline const QString &drmConnector() {
    static const auto s = QStringLiteral("drm_connector");
    return s;
}
inline const QString &drmMinor() {
    static const auto s = QStringLiteral("drm_minor");
    return s;
}
inline const QString &host() {
    static const auto s = QStringLiteral("host");
    return s;
}
inline const QString &link() {
    static const auto s = QStringLiteral("link");
    return s;
}
inline const QString &mfdDevice() {
    static const auto s = QStringLiteral("mfd_device");
    return s;
}
inline const QString &partition() {
    static const auto s = QStringLiteral("partition");
    return s;
}
inline const QString &pcm() {
    static const auto s = QStringLiteral("pcm");
    return s;
}
inline const QString &ppp() {
    static const auto s = QStringLiteral("ppp");
    return s;
}
inline const QString &scsiDevice() {
    static const auto s = QStringLiteral("scsi_device");
    return s;
}
inline const QString &scsiHost() {
    static const auto s = QStringLiteral("scsi_host");
    return s;
}
inline const QString &scsiTarget() {
    static const auto s = QStringLiteral("scsi_target");
    return s;
}
inline const QString &thunderboltDevice() {
    static const auto s = QStringLiteral("thunderbolt_device");
    return s;
}
inline const QString &thunderboltDomain() {
    static const auto s = QStringLiteral("thunderbolt_domain");
    return s;
}
inline const QString &usbDevice() {
    static const auto s = QStringLiteral("usb_device");
    return s;
}
inline const QString &usbInterface() {
    static const auto s = QStringLiteral("usb_interface");
    return s;
}
} // namespace devType

/**
 * @brief DRIVER property values.
 */
namespace driver {
inline const QString &battery() {
    static const auto s = QStringLiteral("battery");
    return s;
}
} // namespace driver

/**
 * @brief ID_MODEL_FROM_DATABASE property values.
 */
namespace idModelFromDatabase {
inline const QString &ups() {
    static const auto s = QStringLiteral("Uninterruptible Power Supply");
    return s;
}
} // namespace idModelFromDatabase

/**
 * @brief ID_PCI_CLASS_FROM_DATABASE property values.
 */
namespace idPciClassFromDatabase {
inline const QString &bridge() {
    static const auto s = QStringLiteral("bridge");
    return s;
}
inline const QString &communicationController() {
    static const auto s = QStringLiteral("Communication controller");
    return s;
}
inline const QString &displayController() {
    static const auto s = QStringLiteral("Display controller");
    return s;
}
inline const QString &genericSystemPeripheral() {
    static const auto s = QStringLiteral("Generic system peripheral");
    return s;
}
inline const QString &massStorageController() {
    static const auto s = QStringLiteral("Mass storage controller");
    return s;
}
inline const QString &memoryController() {
    static const auto s = QStringLiteral("Memory controller");
    return s;
}
inline const QString &multimediaController() {
    static const auto s = QStringLiteral("Multimedia controller");
    return s;
}
inline const QString &networkController() {
    static const auto s = QStringLiteral("Network controller");
    return s;
}
inline const QString &serialBusController() {
    static const auto s = QStringLiteral("Serial bus controller");
    return s;
}
} // namespace idPciClassFromDatabase

/**
 * @brief ID_PCI_INTERFACE_FROM_DATABASE property values.
 */
namespace idPciInterfaceFromDatabase {
inline const QString &ahci1_0() {
    static const auto s = QStringLiteral("AHCI 1.0");
    return s;
}
inline const QString &normalDecode() {
    static const auto s = QStringLiteral("Normal decode");
    return s;
}
inline const QString &nvmExpress() {
    static const auto s = QStringLiteral("NVM Express");
    return s;
}
inline const QString &usb4HostInterface() {
    static const auto s = QStringLiteral("USB4 Host Interface");
    return s;
}
inline const QString &vgaController() {
    static const auto s = QStringLiteral("VGA controller");
    return s;
}
inline const QString &xhci() {
    static const auto s = QStringLiteral("XHCI");
    return s;
}
} // namespace idPciInterfaceFromDatabase

/**
 * @brief ID_PCI_SUBCLASS_FROM_DATABASE property values.
 */
namespace idPciSubclassFromDatabase {
inline const QString &audioDevice() {
    static const auto s = QStringLiteral("Audio device");
    return s;
}
inline const QString &communicationController() {
    static const auto s = QStringLiteral("Communication controller");
    return s;
}
inline const QString &ethernetController() {
    static const auto s = QStringLiteral("Ethernet controller");
    return s;
}
inline const QString &hostBridge() {
    static const auto s = QStringLiteral("Host bridge");
    return s;
}
inline const QString &isaBridge() {
    static const auto s = QStringLiteral("ISA bridge");
    return s;
}
inline const QString &multimediaVideoController() {
    static const auto s = QStringLiteral("Multimedia video controller");
    return s;
}
inline const QString &nonVolatileMemoryController() {
    static const auto s = QStringLiteral("Non-Volatile memory controller");
    return s;
}
inline const QString &pciBridge() {
    static const auto s = QStringLiteral("PCI bridge");
    return s;
}
inline const QString &ramMemory() {
    static const auto s = QStringLiteral("RAM memory");
    return s;
}
inline const QString &sataController() {
    static const auto s = QStringLiteral("SATA controller");
    return s;
}
inline const QString &serialBusController() {
    static const auto s = QStringLiteral("Serial bus controller");
    return s;
}
inline const QString &smBus() {
    static const auto s = QStringLiteral("SMBus");
    return s;
}
inline const QString &systemPeripheral() {
    static const auto s = QStringLiteral("systemPeripheral");
    return s;
}
inline const QString &usbController() {
    static const auto s = QStringLiteral("USB controller");
    return s;
}
inline const QString &vgaCompatibleController() {
    static const auto s = QStringLiteral("VGA compatible controller");
    return s;
}
} // namespace idPciSubclassFromDatabase

/**
 * @brief ID_TYPE property values.
 */
namespace idType {
inline const QString &audio() {
    static const auto s = QStringLiteral("audio");
    return s;
}
} // namespace idType

} // namespace propertyValues

/**
 * @brief udev subsystem name constants.
 */
namespace subsystems {
inline const QString &acpi() {
    static const auto s = QStringLiteral("acpi");
    return s;
}
inline const QString &ataDevice() {
    static const auto s = QStringLiteral("ata_device");
    return s;
}
inline const QString &ataLink() {
    static const auto s = QStringLiteral("ata_link");
    return s;
}
inline const QString &ataPort() {
    static const auto s = QStringLiteral("ata_port");
    return s;
}
inline const QString &battery() {
    static const auto s = QStringLiteral("battery");
    return s;
}
inline const QString &bdi() {
    static const auto s = QStringLiteral("bdi");
    return s;
}
inline const QString &block() {
    static const auto s = QStringLiteral("block");
    return s;
}
inline const QString &bluetooth() {
    static const auto s = QStringLiteral("bluetooth");
    return s;
}
inline const QString &bsg() {
    static const auto s = QStringLiteral("bsg");
    return s;
}
inline const QString &clockEvents() {
    static const auto s = QStringLiteral("clockevents");
    return s;
}
inline const QString &clockSource() {
    static const auto s = QStringLiteral("clocksource");
    return s;
}
inline const QString &container() {
    static const auto s = QStringLiteral("container");
    return s;
}
inline const QString &cpu() {
    static const auto s = QStringLiteral("cpu");
    return s;
}
inline const QString &cpuId() {
    static const auto s = QStringLiteral("cpuid");
    return s;
}
inline const QString &devLink() {
    static const auto s = QStringLiteral("devlink");
    return s;
}
inline const QString &dmi() {
    static const auto s = QStringLiteral("dmi");
    return s;
}
inline const QString &drm() {
    static const auto s = QStringLiteral("drm");
    return s;
}
inline const QString &eventSource() {
    static const auto s = QStringLiteral("event_source");
    return s;
}
inline const QString &graphics() {
    static const auto s = QStringLiteral("graphics");
    return s;
}
inline const QString &hdAudio() {
    static const auto s = QStringLiteral("hdaudio");
    return s;
}
inline const QString &hid() {
    static const auto s = QStringLiteral("hid");
    return s;
}
inline const QString &hidRaw() {
    static const auto s = QStringLiteral("hidraw");
    return s;
}
inline const QString &hwMon() {
    static const auto s = QStringLiteral("hwmon");
    return s;
}
inline const QString &i2c() {
    static const auto s = QStringLiteral("i2c");
    return s;
}
inline const QString &i2cDev() {
    static const auto s = QStringLiteral("i2c-dev");
    return s;
}
inline const QString &input() {
    static const auto s = QStringLiteral("input");
    return s;
}
inline const QString &iommu() {
    static const auto s = QStringLiteral("iommu");
    return s;
}
inline const QString &leds() {
    static const auto s = QStringLiteral("leds");
    return s;
}
inline const QString &machineCheck() {
    static const auto s = QStringLiteral("machinecheck");
    return s;
}
inline const QString &mdioBus() {
    static const auto s = QStringLiteral("mdio_bus");
    return s;
}
inline const QString &media() {
    static const auto s = QStringLiteral("media");
    return s;
}
inline const QString &mei() {
    static const auto s = QStringLiteral("mei");
    return s;
}
inline const QString &mem() {
    static const auto s = QStringLiteral("mem");
    return s;
}
inline const QString &misc() {
    static const auto s = QStringLiteral("misc");
    return s;
}
inline const QString &msr() {
    static const auto s = QStringLiteral("msr");
    return s;
}
inline const QString &mwEvent() {
    static const auto s = QStringLiteral("mwevent");
    return s;
}
inline const QString &net() {
    static const auto s = QStringLiteral("net");
    return s;
}
inline const QString &nvme() {
    static const auto s = QStringLiteral("nvme");
    return s;
}
inline const QString &nvmeGeneric() {
    static const auto s = QStringLiteral("nvme-generic");
    return s;
}
inline const QString &nvMem() {
    static const auto s = QStringLiteral("nvmem");
    return s;
}
inline const QString &nvmeSubsystem() {
    static const auto s = QStringLiteral("nvme-subsystem");
    return s;
}
inline const QString &pci() {
    static const auto s = QStringLiteral("pci");
    return s;
}
inline const QString &pciBus() {
    static const auto s = QStringLiteral("pci_bus");
    return s;
}
inline const QString &pciExpress() {
    static const auto s = QStringLiteral("pci_express");
    return s;
}
inline const QString &platform() {
    static const auto s = QStringLiteral("platform");
    return s;
}
inline const QString &powerSupply() {
    static const auto s = QStringLiteral("power_supply");
    return s;
}
inline const QString &pnp() {
    static const auto s = QStringLiteral("pnp");
    return s;
}
inline const QString &ppp() {
    static const auto s = QStringLiteral("ppp");
    return s;
}
inline const QString &pps() {
    static const auto s = QStringLiteral("pps");
    return s;
}
inline const QString &ptp() {
    static const auto s = QStringLiteral("ptp");
    return s;
}
inline const QString &rfKill() {
    static const auto s = QStringLiteral("rfkill");
    return s;
}
inline const QString &rtc() {
    static const auto s = QStringLiteral("rtc");
    return s;
}
inline const QString &scsi() {
    static const auto s = QStringLiteral("scsi");
    return s;
}
inline const QString &scsiDevice() {
    static const auto s = QStringLiteral("scsi_device");
    return s;
}
inline const QString &scsiDisk() {
    static const auto s = QStringLiteral("scsi_disk");
    return s;
}
inline const QString &scsiGeneric() {
    static const auto s = QStringLiteral("scsi_generic");
    return s;
}
inline const QString &scsiHost() {
    static const auto s = QStringLiteral("scsi_host");
    return s;
}
inline const QString &sound() {
    static const auto s = QStringLiteral("sound");
    return s;
}
inline const QString &thermal() {
    static const auto s = QStringLiteral("thermal");
    return s;
}
inline const QString &thunderbolt() {
    static const auto s = QStringLiteral("thunderbolt");
    return s;
}
inline const QString &tpm() {
    static const auto s = QStringLiteral("tpm");
    return s;
}
inline const QString &tpmRm() {
    static const auto s = QStringLiteral("tpmrm");
    return s;
}
inline const QString &tty() {
    static const auto s = QStringLiteral("tty");
    return s;
}
inline const QString &usb() {
    static const auto s = QStringLiteral("usb");
    return s;
}
inline const QString &usbMisc() {
    static const auto s = QStringLiteral("usbmisc");
    return s;
}
inline const QString &usbMon() {
    static const auto s = QStringLiteral("usbmon");
    return s;
}
inline const QString &vc() {
    static const auto s = QStringLiteral("vc");
    return s;
}
inline const QString &video4Linux() {
    static const auto s = QStringLiteral("video4linux");
    return s;
}
inline const QString &vtConsole() {
    static const auto s = QStringLiteral("vtconsole");
    return s;
}
inline const QString &wakeup() {
    static const auto s = QStringLiteral("wakeup");
    return s;
}
inline const QString &workqueue() {
    static const auto s = QStringLiteral("workqueue");
    return s;
}
} // namespace subsystems

} // namespace udev
} // namespace strings
