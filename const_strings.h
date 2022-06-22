#ifndef CONST_STRINGS_H
#define CONST_STRINGS_H

#include <QtCore/QString>

namespace strings {
    const QString digit1 = QStringLiteral("1");
    const QString empty = QStringLiteral("");
    const QString singleSpace = QStringLiteral(" ");
    const QString underscore = QStringLiteral("_");
    namespace categoryIcons {
        const QString audioInputs = QStringLiteral("audio-card");
        const QString batteries = QStringLiteral("battery-ups");
        const QString computer = QStringLiteral("computer");
        const QString diskDrives = QStringLiteral("drive-harddisk");
        const QString displayAdapters = QStringLiteral("video-display");
        const QString dvdCdromDrives = QStringLiteral("drive-optical");
        const QString hid = QStringLiteral("input-tablet");
        const QString ideAtapiControllers = QStringLiteral("drive-harddisk");
        const QString keyboards = QStringLiteral("input-keyboard");
        const QString mice = QStringLiteral("input-mouse");
        const QString monitor = QStringLiteral("monitor");
        const QString networkAdapters = QStringLiteral("network-wired");
        const QString other = QStringLiteral("preferences-other");
        const QString printer = QStringLiteral("printer");
        const QString processors = QStringLiteral("preferences-devices-cpu");
        const QString soundVideoGameControllers = QStringLiteral("preferences-desktop-sound");
        const QString storageControllers = QStringLiteral("drive-harddisk");
        const QString storageVolumes = QStringLiteral("drive-partition");
        const QString systemDevices = QStringLiteral("computer");
        const QString usbControllers = QStringLiteral("drive-removable-media-usb");
    } // namespace categoryIcons

    namespace udev {
        namespace propertyNames {
            constexpr char DEVNAME[] = "DEVNAME";
            constexpr char DEVPATH[] = "DEVPATH";
            constexpr char DEVTYPE[] = "DEVTYPE";
            constexpr char DM_NAME[] = "DM_NAME";
            constexpr char DRIVER[] = "DRIVER";
            constexpr char HID_NAME[] = "HID_NAME";
            constexpr char HID_ID[] = "HID_ID";
            constexpr char HID_PHYS[] = "HID_PHYS";
            constexpr char HID_UNIQ[] = "HID_UNIQ";
            constexpr char ID_CDROM[] = "ID_CDROM";
            constexpr char ID_FS_LABEL[] = "ID_FS_LABEL";
            constexpr char ID_PART_ENTRY_NAME[] = "ID_PART_ENTRY_NAME";
            constexpr char ID_PART_TABLE_UUID[] = "ID_PART_TABLE_UUID";
            constexpr char ID_INPUT_KEYBOARD[] = "ID_INPUT_KEYBOARD";
            constexpr char ID_INPUT_MOUSE[] = "ID_INPUT_MOUSE";
            constexpr char ID_MODEL[] = "ID_MODEL";
            constexpr char ID_MODEL_FROM_DATABASE[] = "ID_MODEL_FROM_DATABASE";
            constexpr char ID_PCI_CLASS_FROM_DATABASE[] = "ID_PCI_CLASS_FROM_DATABASE";
            constexpr char ID_PCI_INTERFACE_FROM_DATABASE[] = "ID_PCI_INTERFACE_FROM_DATABASE";
            constexpr char ID_PCI_SUBCLASS_FROM_DATABASE[] = "ID_PCI_SUBCLASS_FROM_DATABASE";
            constexpr char ID_TYPE[] = "ID_TYPE";
            constexpr char ID_VENDOR_FROM_DATABASE[] = "ID_VENDOR_FROM_DATABASE";
            constexpr char MODALIAS[] = "MODALIAS";
            constexpr char NAME[] = "NAME";
            constexpr char SUBSYSTEM[] = "SUBSYSTEM";
        } // namespace propertyNames

        namespace propertyValues {
            namespace devType {
                const QString disk = QStringLiteral("disk");
                const QString drmConnector = QStringLiteral("drm_connector");
                const QString drmMinor = QStringLiteral("drm_minor");
                const QString host = QStringLiteral("host");
                const QString link = QStringLiteral("link");
                const QString mfdDevice = QStringLiteral("mfd_device");
                const QString partition = QStringLiteral("partition");
                const QString pcm = QStringLiteral("pcm");
                const QString ppp = QStringLiteral("ppp");
                const QString scsiDevice = QStringLiteral("scsi_device");
                const QString scsiHost = QStringLiteral("scsi_host");
                const QString scsiTarget = QStringLiteral("scsi_target");
                const QString thunderboltDevice = QStringLiteral("thunderbolt_device");
                const QString thunderboltDomain = QStringLiteral("thunderbolt_domain");
                const QString usbDevice = QStringLiteral("usb_device");
                const QString usbInterface = QStringLiteral("usb_interface");
            } // namespace devType

            namespace driver {
                const QString battery = QStringLiteral("battery");
            }

            namespace idModelFromDatabase {
                const QString ups = QStringLiteral("Uninterruptible Power Supply");
            }

            namespace idPciClassFromDatabase {
                const QString bridge = QStringLiteral("bridge");
                const QString communicationController = QStringLiteral("Communication controller");
                const QString displayController = QStringLiteral("Display controller");
                const QString genericSystemPeripheral = QStringLiteral("Generic system peripheral");
                const QString massStorageController = QStringLiteral("Mass storage controller");
                const QString memoryController = QStringLiteral("Memory controller");
                const QString multimediaController = QStringLiteral("Multimedia controller");
                const QString networkController = QStringLiteral("Network controller");
                const QString serialBusController = QStringLiteral("Serial bus controller");
            } // namespace idPciClassFromDatabase

            namespace idPciInterfaceFromDatabase {
                const QString ahci1_0 = QStringLiteral("AHCI 1.0");
                const QString normalDecode = QStringLiteral("Normal decode");
                const QString nvmExpress = QStringLiteral("NVM Express");
                const QString usb4HostInterface = QStringLiteral("USB4 Host Interface");
                const QString vgaController = QStringLiteral("VGA controller");
                const QString xhci = QStringLiteral("XHCI");
            } // namespace idPciInterfaceFromDatabase

            namespace idPciSubclassFromDatabase {
                const QString audioDevice = QStringLiteral("Audio device");
                const QString communicationController = QStringLiteral("Communication controller");
                const QString ethernetController = QStringLiteral("Ethernet controller");
                const QString hostBridge = QStringLiteral("Host bridge");
                const QString isaBridge = QStringLiteral("ISA bridge");
                const QString multimediaVideoController =
                    QStringLiteral("Multimedia video controller");
                const QString nonVolatileMemoryController =
                    QStringLiteral("Non-Volatile memory controller");
                const QString pciBridge = QStringLiteral("PCI bridge");
                const QString ramMemory = QStringLiteral("RAM memory");
                const QString sataController = QStringLiteral("SATA controller");
                const QString serialBusController = QStringLiteral("Serial bus controller");
                const QString smBus = QStringLiteral("SMBus");
                const QString systemPeripheral = QStringLiteral("systemPeripheral");
                const QString usbController = QStringLiteral("USB controller");
                const QString vgaCompatibleController = QStringLiteral("VGA compatible controller");
            } // namespace idPciSubclassFromDatabase

            namespace idType {
                const QString audio = QStringLiteral("audio");
            }
        } // namespace propertyValues

        namespace subsystems {
            const QString acpi = QStringLiteral("acpi");
            const QString ataDevice = QStringLiteral("ata_device");
            const QString ataLink = QStringLiteral("ata_link");
            const QString ataPort = QStringLiteral("ata_port");
            const QString battery = QStringLiteral("battery");
            const QString bdi = QStringLiteral("bdi");
            const QString block = QStringLiteral("block");
            const QString bluetooth = QStringLiteral("bluetooth");
            const QString bsg = QStringLiteral("bsg");
            const QString clockEvents = QStringLiteral("clockevents");
            const QString clockSource = QStringLiteral("clocksource");
            const QString container = QStringLiteral("container");
            const QString cpu = QStringLiteral("cpu");
            const QString cpuId = QStringLiteral("cpuid");
            const QString devLink = QStringLiteral("devlink");
            const QString dmi = QStringLiteral("dmi");
            const QString drm = QStringLiteral("drm");
            const QString eventSource = QStringLiteral("event_source");
            const QString graphics = QStringLiteral("graphics");
            const QString hdAudio = QStringLiteral("hdaudio");
            const QString hid = QStringLiteral("hid");
            const QString hidRaw = QStringLiteral("hidraw");
            const QString hwMon = QStringLiteral("hwmon");
            const QString i2c = QStringLiteral("i2c");
            const QString i2cDev = QStringLiteral("i2c-dev");
            const QString input = QStringLiteral("input");
            const QString iommu = QStringLiteral("iommu");
            const QString leds = QStringLiteral("leds");
            const QString machineCheck = QStringLiteral("machinecheck");
            const QString mdioBus = QStringLiteral("mdio_bus");
            const QString media = QStringLiteral("media");
            const QString mei = QStringLiteral("mei");
            const QString mem = QStringLiteral("mem");
            const QString misc = QStringLiteral("misc");
            const QString msr = QStringLiteral("msr");
            const QString mwEvent = QStringLiteral("mwevent");
            const QString net = QStringLiteral("net");
            const QString nvme = QStringLiteral("nvme");
            const QString nvmeGeneric = QStringLiteral("nvme-generic");
            const QString nvMem = QStringLiteral("nvmem");
            const QString nvmeSubsystem = QStringLiteral("nvme-subsystem");
            const QString pci = QStringLiteral("pci");
            const QString pciBus = QStringLiteral("pci_bus");
            const QString pciExpress = QStringLiteral("pci_express");
            const QString platform = QStringLiteral("platform");
            const QString powerSupply = QStringLiteral("power_supply");
            const QString pnp = QStringLiteral("pnp");
            const QString ppp = QStringLiteral("ppp");
            const QString pps = QStringLiteral("pps");
            const QString ptp = QStringLiteral("ptp");
            const QString rfKill = QStringLiteral("rfkill");
            const QString rtc = QStringLiteral("rtc");
            const QString scsi = QStringLiteral("scsi");
            const QString scsiDevice = QStringLiteral("scsi_device");
            const QString scsiDisk = QStringLiteral("scsi_disk");
            const QString scsiGeneric = QStringLiteral("scsi_generic");
            const QString scsiHost = QStringLiteral("scsi_host");
            const QString sound = QStringLiteral("sound");
            const QString thermal = QStringLiteral("thermal");
            const QString thunderbolt = QStringLiteral("thunderbolt");
            const QString tpm = QStringLiteral("tpm");
            const QString tpmRm = QStringLiteral("tpmrm");
            const QString tty = QStringLiteral("tty");
            const QString usb = QStringLiteral("usb");
            const QString usbMisc = QStringLiteral("usbmisc");
            const QString usbMon = QStringLiteral("usbmon");
            const QString vc = QStringLiteral("vc");
            const QString video4Linux = QStringLiteral("video4linux");
            const QString vtConsole = QStringLiteral("vtconsole");
            const QString wakeup = QStringLiteral("wakeup");
            const QString workqueue = QStringLiteral("workqueue");

        } // namespace subsystems
    }     // namespace udev
} // namespace strings

#endif // CONST_STRINGS_H
