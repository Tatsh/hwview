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
        const QString soundVideoGameControllers =
            QStringLiteral("preferences-desktop-sound");
        const QString storageControllers = QStringLiteral("drive-harddisk");
        const QString storageVolumes = QStringLiteral("drive-partition");
        const QString systemDevices = QStringLiteral("computer");
        const QString usbControllers =
            QStringLiteral("drive-removable-media-usb");
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
            constexpr char ID_PCI_CLASS_FROM_DATABASE[] =
                "ID_PCI_CLASS_FROM_DATABASE";
            constexpr char ID_PCI_INTERFACE_FROM_DATABASE[] =
                "ID_PCI_INTERFACE_FROM_DATABASE";
            constexpr char ID_PCI_SUBCLASS_FROM_DATABASE[] =
                "ID_PCI_SUBCLASS_FROM_DATABASE";
            constexpr char ID_VENDOR_FROM_DATABASE[] =
                "ID_VENDOR_FROM_DATABASE";
            constexpr char MODALIAS[] = "MODALIAS";
            constexpr char NAME[] = "NAME";
            constexpr char SUBSYSTEM[] = "SUBSYSTEM";
        } // namespace propertyNames

        namespace propertyValues {
            namespace devType {
                const QString partition = QStringLiteral("partition");
            }

            namespace idPciClassFromDatabase {
                const QString displayController =
                    QStringLiteral("Display controller");
                const QString massStorageController =
                    QStringLiteral("Mass storage controller");
                const QString multimediaController =
                    QStringLiteral("Multimedia controller");
                const QString networkController =
                    QStringLiteral("Network controller");
            } // namespace idPciClassFromDatabase

            namespace idPciInterfaceFromDatabase {
                const QString nvmExpress = QStringLiteral("NVM Express");
            }

            namespace idPciSubclassFromDatabase {
                const QString audioDevice = QStringLiteral("Audio device");
                const QString sataController =
                    QStringLiteral("SATA controller");
                const QString usbController = QStringLiteral("USB controller");
            } // namespace idPciSubclassFromDatabase
        }     // namespace propertyValues

        namespace subsystems {
            const QString block = QStringLiteral("block");
            const QString hid = QStringLiteral("hid");
            const QString misc = QStringLiteral("misc");
            const QString pci = QStringLiteral("pci");
        } // namespace subsystems
    }     // namespace udev
} // namespace strings

#endif // CONST_STRINGS_H
