#pragma once

#include <QString>

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
    static const QString s = QStringLiteral("disk");
    return s;
}
inline const QString &drmConnector() {
    static const QString s = QStringLiteral("drm_connector");
    return s;
}
inline const QString &drmMinor() {
    static const QString s = QStringLiteral("drm_minor");
    return s;
}
inline const QString &host() {
    static const QString s = QStringLiteral("host");
    return s;
}
inline const QString &link() {
    static const QString s = QStringLiteral("link");
    return s;
}
inline const QString &mfdDevice() {
    static const QString s = QStringLiteral("mfd_device");
    return s;
}
inline const QString &partition() {
    static const QString s = QStringLiteral("partition");
    return s;
}
inline const QString &pcm() {
    static const QString s = QStringLiteral("pcm");
    return s;
}
inline const QString &ppp() {
    static const QString s = QStringLiteral("ppp");
    return s;
}
inline const QString &scsiDevice() {
    static const QString s = QStringLiteral("scsi_device");
    return s;
}
inline const QString &scsiHost() {
    static const QString s = QStringLiteral("scsi_host");
    return s;
}
inline const QString &scsiTarget() {
    static const QString s = QStringLiteral("scsi_target");
    return s;
}
inline const QString &thunderboltDevice() {
    static const QString s = QStringLiteral("thunderbolt_device");
    return s;
}
inline const QString &thunderboltDomain() {
    static const QString s = QStringLiteral("thunderbolt_domain");
    return s;
}
inline const QString &usbDevice() {
    static const QString s = QStringLiteral("usb_device");
    return s;
}
inline const QString &usbInterface() {
    static const QString s = QStringLiteral("usb_interface");
    return s;
}
} // namespace devType

/**
 * @brief DRIVER property values.
 */
namespace driver {
inline const QString &battery() {
    static const QString s = QStringLiteral("battery");
    return s;
}
} // namespace driver

/**
 * @brief ID_MODEL_FROM_DATABASE property values.
 */
namespace idModelFromDatabase {
inline const QString &ups() {
    static const QString s = QStringLiteral("Uninterruptible Power Supply");
    return s;
}
} // namespace idModelFromDatabase

/**
 * @brief ID_PCI_CLASS_FROM_DATABASE property values.
 */
namespace idPciClassFromDatabase {
inline const QString &bridge() {
    static const QString s = QStringLiteral("bridge");
    return s;
}
inline const QString &communicationController() {
    static const QString s = QStringLiteral("Communication controller");
    return s;
}
inline const QString &displayController() {
    static const QString s = QStringLiteral("Display controller");
    return s;
}
inline const QString &genericSystemPeripheral() {
    static const QString s = QStringLiteral("Generic system peripheral");
    return s;
}
inline const QString &massStorageController() {
    static const QString s = QStringLiteral("Mass storage controller");
    return s;
}
inline const QString &memoryController() {
    static const QString s = QStringLiteral("Memory controller");
    return s;
}
inline const QString &multimediaController() {
    static const QString s = QStringLiteral("Multimedia controller");
    return s;
}
inline const QString &networkController() {
    static const QString s = QStringLiteral("Network controller");
    return s;
}
inline const QString &serialBusController() {
    static const QString s = QStringLiteral("Serial bus controller");
    return s;
}
} // namespace idPciClassFromDatabase

/**
 * @brief ID_PCI_INTERFACE_FROM_DATABASE property values.
 */
namespace idPciInterfaceFromDatabase {
inline const QString &ahci1_0() {
    static const QString s = QStringLiteral("AHCI 1.0");
    return s;
}
inline const QString &normalDecode() {
    static const QString s = QStringLiteral("Normal decode");
    return s;
}
inline const QString &nvmExpress() {
    static const QString s = QStringLiteral("NVM Express");
    return s;
}
inline const QString &usb4HostInterface() {
    static const QString s = QStringLiteral("USB4 Host Interface");
    return s;
}
inline const QString &vgaController() {
    static const QString s = QStringLiteral("VGA controller");
    return s;
}
inline const QString &xhci() {
    static const QString s = QStringLiteral("XHCI");
    return s;
}
} // namespace idPciInterfaceFromDatabase

/**
 * @brief ID_PCI_SUBCLASS_FROM_DATABASE property values.
 */
namespace idPciSubclassFromDatabase {
inline const QString &audioDevice() {
    static const QString s = QStringLiteral("Audio device");
    return s;
}
inline const QString &communicationController() {
    static const QString s = QStringLiteral("Communication controller");
    return s;
}
inline const QString &ethernetController() {
    static const QString s = QStringLiteral("Ethernet controller");
    return s;
}
inline const QString &hostBridge() {
    static const QString s = QStringLiteral("Host bridge");
    return s;
}
inline const QString &isaBridge() {
    static const QString s = QStringLiteral("ISA bridge");
    return s;
}
inline const QString &multimediaVideoController() {
    static const QString s = QStringLiteral("Multimedia video controller");
    return s;
}
inline const QString &nonVolatileMemoryController() {
    static const QString s = QStringLiteral("Non-Volatile memory controller");
    return s;
}
inline const QString &pciBridge() {
    static const QString s = QStringLiteral("PCI bridge");
    return s;
}
inline const QString &ramMemory() {
    static const QString s = QStringLiteral("RAM memory");
    return s;
}
inline const QString &sataController() {
    static const QString s = QStringLiteral("SATA controller");
    return s;
}
inline const QString &serialBusController() {
    static const QString s = QStringLiteral("Serial bus controller");
    return s;
}
inline const QString &smBus() {
    static const QString s = QStringLiteral("SMBus");
    return s;
}
inline const QString &systemPeripheral() {
    static const QString s = QStringLiteral("systemPeripheral");
    return s;
}
inline const QString &usbController() {
    static const QString s = QStringLiteral("USB controller");
    return s;
}
inline const QString &vgaCompatibleController() {
    static const QString s = QStringLiteral("VGA compatible controller");
    return s;
}
} // namespace idPciSubclassFromDatabase

/**
 * @brief ID_TYPE property values.
 */
namespace idType {
inline const QString &audio() {
    static const QString s = QStringLiteral("audio");
    return s;
}
} // namespace idType

} // namespace propertyValues

/**
 * @brief udev subsystem name constants.
 */
namespace subsystems {
inline const QString &acpi() {
    static const QString s = QStringLiteral("acpi");
    return s;
}
inline const QString &ataDevice() {
    static const QString s = QStringLiteral("ata_device");
    return s;
}
inline const QString &ataLink() {
    static const QString s = QStringLiteral("ata_link");
    return s;
}
inline const QString &ataPort() {
    static const QString s = QStringLiteral("ata_port");
    return s;
}
inline const QString &battery() {
    static const QString s = QStringLiteral("battery");
    return s;
}
inline const QString &bdi() {
    static const QString s = QStringLiteral("bdi");
    return s;
}
inline const QString &block() {
    static const QString s = QStringLiteral("block");
    return s;
}
inline const QString &bluetooth() {
    static const QString s = QStringLiteral("bluetooth");
    return s;
}
inline const QString &bsg() {
    static const QString s = QStringLiteral("bsg");
    return s;
}
inline const QString &clockEvents() {
    static const QString s = QStringLiteral("clockevents");
    return s;
}
inline const QString &clockSource() {
    static const QString s = QStringLiteral("clocksource");
    return s;
}
inline const QString &container() {
    static const QString s = QStringLiteral("container");
    return s;
}
inline const QString &cpu() {
    static const QString s = QStringLiteral("cpu");
    return s;
}
inline const QString &cpuId() {
    static const QString s = QStringLiteral("cpuid");
    return s;
}
inline const QString &devLink() {
    static const QString s = QStringLiteral("devlink");
    return s;
}
inline const QString &dmi() {
    static const QString s = QStringLiteral("dmi");
    return s;
}
inline const QString &drm() {
    static const QString s = QStringLiteral("drm");
    return s;
}
inline const QString &eventSource() {
    static const QString s = QStringLiteral("event_source");
    return s;
}
inline const QString &graphics() {
    static const QString s = QStringLiteral("graphics");
    return s;
}
inline const QString &hdAudio() {
    static const QString s = QStringLiteral("hdaudio");
    return s;
}
inline const QString &hid() {
    static const QString s = QStringLiteral("hid");
    return s;
}
inline const QString &hidRaw() {
    static const QString s = QStringLiteral("hidraw");
    return s;
}
inline const QString &hwMon() {
    static const QString s = QStringLiteral("hwmon");
    return s;
}
inline const QString &i2c() {
    static const QString s = QStringLiteral("i2c");
    return s;
}
inline const QString &i2cDev() {
    static const QString s = QStringLiteral("i2c-dev");
    return s;
}
inline const QString &input() {
    static const QString s = QStringLiteral("input");
    return s;
}
inline const QString &iommu() {
    static const QString s = QStringLiteral("iommu");
    return s;
}
inline const QString &leds() {
    static const QString s = QStringLiteral("leds");
    return s;
}
inline const QString &machineCheck() {
    static const QString s = QStringLiteral("machinecheck");
    return s;
}
inline const QString &mdioBus() {
    static const QString s = QStringLiteral("mdio_bus");
    return s;
}
inline const QString &media() {
    static const QString s = QStringLiteral("media");
    return s;
}
inline const QString &mei() {
    static const QString s = QStringLiteral("mei");
    return s;
}
inline const QString &mem() {
    static const QString s = QStringLiteral("mem");
    return s;
}
inline const QString &misc() {
    static const QString s = QStringLiteral("misc");
    return s;
}
inline const QString &msr() {
    static const QString s = QStringLiteral("msr");
    return s;
}
inline const QString &mwEvent() {
    static const QString s = QStringLiteral("mwevent");
    return s;
}
inline const QString &net() {
    static const QString s = QStringLiteral("net");
    return s;
}
inline const QString &nvme() {
    static const QString s = QStringLiteral("nvme");
    return s;
}
inline const QString &nvmeGeneric() {
    static const QString s = QStringLiteral("nvme-generic");
    return s;
}
inline const QString &nvMem() {
    static const QString s = QStringLiteral("nvmem");
    return s;
}
inline const QString &nvmeSubsystem() {
    static const QString s = QStringLiteral("nvme-subsystem");
    return s;
}
inline const QString &pci() {
    static const QString s = QStringLiteral("pci");
    return s;
}
inline const QString &pciBus() {
    static const QString s = QStringLiteral("pci_bus");
    return s;
}
inline const QString &pciExpress() {
    static const QString s = QStringLiteral("pci_express");
    return s;
}
inline const QString &platform() {
    static const QString s = QStringLiteral("platform");
    return s;
}
inline const QString &powerSupply() {
    static const QString s = QStringLiteral("power_supply");
    return s;
}
inline const QString &pnp() {
    static const QString s = QStringLiteral("pnp");
    return s;
}
inline const QString &ppp() {
    static const QString s = QStringLiteral("ppp");
    return s;
}
inline const QString &pps() {
    static const QString s = QStringLiteral("pps");
    return s;
}
inline const QString &ptp() {
    static const QString s = QStringLiteral("ptp");
    return s;
}
inline const QString &rfKill() {
    static const QString s = QStringLiteral("rfkill");
    return s;
}
inline const QString &rtc() {
    static const QString s = QStringLiteral("rtc");
    return s;
}
inline const QString &scsi() {
    static const QString s = QStringLiteral("scsi");
    return s;
}
inline const QString &scsiDevice() {
    static const QString s = QStringLiteral("scsi_device");
    return s;
}
inline const QString &scsiDisk() {
    static const QString s = QStringLiteral("scsi_disk");
    return s;
}
inline const QString &scsiGeneric() {
    static const QString s = QStringLiteral("scsi_generic");
    return s;
}
inline const QString &scsiHost() {
    static const QString s = QStringLiteral("scsi_host");
    return s;
}
inline const QString &sound() {
    static const QString s = QStringLiteral("sound");
    return s;
}
inline const QString &thermal() {
    static const QString s = QStringLiteral("thermal");
    return s;
}
inline const QString &thunderbolt() {
    static const QString s = QStringLiteral("thunderbolt");
    return s;
}
inline const QString &tpm() {
    static const QString s = QStringLiteral("tpm");
    return s;
}
inline const QString &tpmRm() {
    static const QString s = QStringLiteral("tpmrm");
    return s;
}
inline const QString &tty() {
    static const QString s = QStringLiteral("tty");
    return s;
}
inline const QString &usb() {
    static const QString s = QStringLiteral("usb");
    return s;
}
inline const QString &usbMisc() {
    static const QString s = QStringLiteral("usbmisc");
    return s;
}
inline const QString &usbMon() {
    static const QString s = QStringLiteral("usbmon");
    return s;
}
inline const QString &vc() {
    static const QString s = QStringLiteral("vc");
    return s;
}
inline const QString &video4Linux() {
    static const QString s = QStringLiteral("video4linux");
    return s;
}
inline const QString &vtConsole() {
    static const QString s = QStringLiteral("vtconsole");
    return s;
}
inline const QString &wakeup() {
    static const QString s = QStringLiteral("wakeup");
    return s;
}
inline const QString &workqueue() {
    static const QString s = QStringLiteral("workqueue");
    return s;
}
} // namespace subsystems

} // namespace udev
} // namespace strings
