#pragma once

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

        if (shortName == QStringLiteral("autofs"))
            return QStringLiteral("Automount filesystem");
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
} // namespace strings
