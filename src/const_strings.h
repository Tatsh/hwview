#pragma once

#include <QRegularExpression>
#include <QString>

// Include split headers for backward compatibility
#include "const_strings_icons.h"
#include "const_strings_udev.h"

namespace strings {
    // Use inline functions to avoid static initialization order issues
    inline const QString &digit1() {
        static const QString s = QStringLiteral("1");
        return s;
    }
    inline const QString &empty() {
        static const QString s;
        return s;
    }
    inline const QString &singleSpace() {
        static const QString s = QStringLiteral(" ");
        return s;
    }
    inline const QString &underscore() {
        static const QString s = QStringLiteral("_");
        return s;
    }

    // Common HID vendor IDs - must be defined before softwareDeviceNiceName
    inline QString hidVendorName(const QString &vendorId) {
        bool ok;
        int vid = vendorId.toInt(&ok, 16);
        if (!ok) {
            return QString();
        }

        switch (vid) {
        case 0x0002:
            return QStringLiteral("Microsoft");
        case 0x044E:
            return QStringLiteral("Alps Electric");
        case 0x0457:
            return QStringLiteral("Silicon Integrated Systems");
        case 0x046D:
            return QStringLiteral("Logitech");
        case 0x04B4:
            return QStringLiteral("Cypress Semiconductor");
        case 0x04F3:
            return QStringLiteral("Elan Microelectronics");
        case 0x056A:
            return QStringLiteral("Wacom");
        case 0x06CB:
            return QStringLiteral("Synaptics");
        case 0x093A:
            return QStringLiteral("Pixart Imaging");
        case 0x1532:
            return QStringLiteral("Razer");
        case 0x17EF:
            return QStringLiteral("Lenovo");
        case 0x1B96:
            return QStringLiteral("N-Trig");
        case 0x27C6:
            return QStringLiteral("Goodix");
        case 0x2808:
            return QStringLiteral("Focal Tech");
        default:
            return QString();
        }
    }

    // Nice name mapping for software devices
    inline QString softwareDeviceNiceName(const QString &name) {
        // Strip /dev/ prefix if present
        QString shortName = name;
        if (shortName.startsWith(QStringLiteral("/dev/"))) {
            shortName = shortName.mid(5);
        }

        // Handle input/event* devices
        if (shortName.startsWith(QStringLiteral("input/event"))) {
            QString num = shortName.mid(11); // Get the number after "input/event"
            return QStringLiteral("Input event %1").arg(num);
        }
        // Handle input/mouse* devices
        if (shortName.startsWith(QStringLiteral("input/mouse"))) {
            QString num = shortName.mid(11); // Get the number after "input/mouse"
            return QStringLiteral("Input mouse %1").arg(num);
        }

        // Fix ugly device names
        if (shortName == QStringLiteral("Metadot - Das Keyboard Das Keyboard") ||
            shortName == QStringLiteral("Metadot_-_Das_Keyboard_Das_Keyboard")) {
            return QStringLiteral("Das Keyboard");
        }

        // Parse HID device names like "PNP0C50:00 06CB:7E7E Mouse"
        // Format: PNP_ID:INSTANCE VENDOR:PRODUCT DeviceType
        static const QRegularExpression hidNameRe(
            QStringLiteral("^(?:PNP[0-9A-Fa-f]{4}|ACPI[0-9A-Fa-f]{4}):[0-9A-Fa-f]+\\s+"
                           "([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4})(?:\\s+(.+))?$"));
        auto hidMatch = hidNameRe.match(shortName);
        if (hidMatch.hasMatch()) {
            QString vendorId = hidMatch.captured(1);
            QString deviceType = hidMatch.captured(3).trimmed();
            QString vendorName = hidVendorName(vendorId);

            if (!vendorName.isEmpty()) {
                if (!deviceType.isEmpty()) {
                    return QStringLiteral("%1 %2").arg(vendorName, deviceType);
                }
                return QStringLiteral("%1 HID device").arg(vendorName);
            }
            // If vendor unknown, just show the device type or a generic name
            if (!deviceType.isEmpty()) {
                return deviceType;
            }
        }

        if (shortName == QStringLiteral("acpi thermal rel") ||
            shortName == QStringLiteral("acpi_thermal_rel"))
            return QStringLiteral("ACPI thermal relationship");
        if (shortName == QStringLiteral("autofs"))
            return QStringLiteral("Automount filesystem");
        if (shortName == QStringLiteral("btrfs-control"))
            return QStringLiteral("Btrfs control");
        if (shortName == QStringLiteral("cpu dma latency") ||
            shortName == QStringLiteral("cpu_dma_latency"))
            return QStringLiteral("CPU DMA latency");
        if (shortName == QStringLiteral("fuse"))
            return QStringLiteral("FUSE");
        if (shortName == QStringLiteral("hpet"))
            return QStringLiteral("HPET");
        if (shortName == QStringLiteral("hwrng"))
            return QStringLiteral("Hardware random number generator");
        if (shortName == QStringLiteral("kvm"))
            return QStringLiteral("KVM");
        if (shortName == QStringLiteral("loop-control") ||
            shortName == QStringLiteral("loop control"))
            return QStringLiteral("Loop device control");
        if (shortName == QStringLiteral("mcelog"))
            return QStringLiteral("MCE log");
        if (shortName == QStringLiteral("net/tun"))
            return QStringLiteral("TUN/TAP network device");
        if (shortName == QStringLiteral("ntsync"))
            return QStringLiteral("NT synchronization primitives");
        if (shortName == QStringLiteral("rfkill"))
            return QStringLiteral("RF kill switch");
        if (shortName == QStringLiteral("uhid"))
            return QStringLiteral("User-space HID");
        if (shortName == QStringLiteral("uinput"))
            return QStringLiteral("User-space input device");
        if (shortName == QStringLiteral("userfaultfd"))
            return QStringLiteral("User-space page fault handling");
        if (shortName == QStringLiteral("vboxdrv"))
            return QStringLiteral("VirtualBox kernel driver");
        if (shortName == QStringLiteral("vboxdrvu"))
            return QStringLiteral("VirtualBox user driver");
        if (shortName == QStringLiteral("vboxnetctl"))
            return QStringLiteral("VirtualBox network control");
        if (shortName == QStringLiteral("vga arbiter") ||
            shortName == QStringLiteral("vga_arbiter"))
            return QStringLiteral("VGA arbiter");
        if (shortName == QStringLiteral("vhost-net"))
            return QStringLiteral("Virtio host network");
        if (shortName == QStringLiteral("mapper/control"))
            return QStringLiteral("Device mapper control");

        return name; // Return original if no mapping found
    }

    // HID bus type constants (from Linux input.h)
    namespace hidBusTypes {
        constexpr int BUS_PCI = 0x01;
        constexpr int BUS_USB = 0x03;
        constexpr int BUS_BLUETOOTH = 0x05;
        constexpr int BUS_VIRTUAL = 0x06;
        constexpr int BUS_I2C = 0x18;
        constexpr int BUS_HOST = 0x19;
    } // namespace hidBusTypes

    // Parse HID device ID from path component like "0018:06CB:7E7E.0005"
    // Returns struct with bus type, vendor ID, product ID, and instance
    struct HidDeviceId {
        int busType = 0;
        QString vendorId;
        QString productId;
        QString instance;
        bool valid = false;
    };

    inline HidDeviceId parseHidDeviceId(const QString &path) {
        HidDeviceId result;
        // Match pattern like 0018:06CB:7E7E.0005
        static const QRegularExpression hidIdRe(
            QStringLiteral("([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4})\\.([0-9A-Fa-f]+)"));

        for (const QString &component : path.split(QLatin1Char('/'))) {
            auto match = hidIdRe.match(component);
            if (match.hasMatch()) {
                bool ok;
                result.busType = match.captured(1).toInt(&ok, 16);
                result.vendorId = match.captured(2);
                result.productId = match.captured(3);
                result.instance = match.captured(4);
                result.valid = ok;
                return result;
            }
        }
        return result;
    }

    // Get HID bus type name
    inline QString hidBusTypeName(int busType) {
        switch (busType) {
        case hidBusTypes::BUS_PCI:
            return QStringLiteral("PCI");
        case hidBusTypes::BUS_USB:
            return QStringLiteral("USB");
        case hidBusTypes::BUS_BLUETOOTH:
            return QStringLiteral("Bluetooth");
        case hidBusTypes::BUS_VIRTUAL:
            return QStringLiteral("Virtual");
        case hidBusTypes::BUS_I2C:
            return QStringLiteral("I2C HID");
        case hidBusTypes::BUS_HOST:
            return QStringLiteral("Host");
        default:
            return QString();
        }
    }

    // Parse I2C bus number from path
    inline int parseI2cBusNumber(const QString &path) {
        static const QRegularExpression i2cBusRe(QStringLiteral("/i2c-(\\d+)(?:/|$)"));
        auto match = i2cBusRe.match(path);
        if (match.hasMatch()) {
            bool ok;
            int bus = match.captured(1).toInt(&ok);
            if (ok) {
                return bus;
            }
        }
        return -1;
    }

    // Nice name mapping for ACPI/PNP device IDs
    // Path format: /devices/LNXSYSTM:00/LNXSYBUS:00/.../PNP0C0A:00
    inline QString acpiDeviceNiceName(const QString &devPath, const QString &fallbackName) {
        // Extract the last component (e.g., "PNP0C0A:00")
        int lastSlash = devPath.lastIndexOf(QLatin1Char('/'));
        if (lastSlash < 0) {
            return fallbackName;
        }
        QString lastComponent = devPath.mid(lastSlash + 1);

        // Split into PNP ID and instance number
        int colonPos = lastComponent.indexOf(QLatin1Char(':'));
        QString pnpId = colonPos > 0 ? lastComponent.left(colonPos) : lastComponent;
        QString instance = colonPos > 0 ? lastComponent.mid(colonPos + 1) : QString();

        // Map PNP IDs to nice names
        QString niceName;
        if (pnpId == QStringLiteral("PNP0C0A")) {
            niceName = QStringLiteral("Microsoft ACPI-Compliant Control Method Battery");
        } else if (pnpId == QStringLiteral("PNP0C0D")) {
            niceName = QStringLiteral("ACPI Lid");
        } else if (pnpId == QStringLiteral("PNP0C0E")) {
            niceName = QStringLiteral("ACPI Sleep Button");
        } else if (pnpId == QStringLiteral("PNP0C0F")) {
            niceName = QStringLiteral("ACPI PCI Interrupt Link");
        } else if (pnpId == QStringLiteral("PNP0C09")) {
            niceName = QStringLiteral("ACPI Embedded Controller");
        } else if (pnpId == QStringLiteral("PNP0A08") || pnpId == QStringLiteral("PNP0A03")) {
            niceName = QStringLiteral("PCI Express Root Complex");
        } else if (pnpId == QStringLiteral("PNP0C02")) {
            niceName = QStringLiteral("ACPI Motherboard Resources");
        } else if (pnpId == QStringLiteral("PNP0B00")) {
            niceName = QStringLiteral("ACPI Real Time Clock");
        } else if (pnpId == QStringLiteral("PNP0C04")) {
            niceName = QStringLiteral("ACPI Math Coprocessor");
        } else if (pnpId == QStringLiteral("PNP0100")) {
            niceName = QStringLiteral("ACPI System Timer");
        } else if (pnpId == QStringLiteral("PNP0103")) {
            niceName = QStringLiteral("ACPI High Precision Event Timer");
        } else if (pnpId == QStringLiteral("LNXPOWER") || pnpId == QStringLiteral("PNP0C0C")) {
            niceName = QStringLiteral("ACPI Power Button");
        } else if (pnpId == QStringLiteral("LNXSYSTM")) {
            niceName = QStringLiteral("ACPI System");
        } else if (pnpId == QStringLiteral("LNXSYBUS")) {
            niceName = QStringLiteral("ACPI System Bus");
        } else if (pnpId == QStringLiteral("LNXCPU")) {
            niceName = QStringLiteral("ACPI Processor");
        } else if (pnpId == QStringLiteral("LNXTHERM")) {
            niceName = QStringLiteral("ACPI Thermal Zone");
        } else if (pnpId == QStringLiteral("ACPI0003")) {
            niceName = QStringLiteral("Microsoft AC Adapter");
        } else {
            // Capitalize simple names like "battery" -> "Battery"
            if (!fallbackName.isEmpty()) {
                QString result = fallbackName;
                result[0] = result[0].toUpper();
                return result;
            }
            return fallbackName;
        }

        return niceName;
    }
} // namespace strings
