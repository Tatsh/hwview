#pragma once

#include <QIcon>
#include <QString>

namespace strings {
    namespace categoryIcons {
        // Cached icon accessors to avoid repeated theme lookups
        inline const QIcon &audioInputs() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("audio-card"));
            return icon;
        }
        inline const QIcon &batteries() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("battery-ups"));
            return icon;
        }
        inline const QIcon &computer() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("computer"));
            return icon;
        }
        inline const QIcon &diskDrives() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
            return icon;
        }
        inline const QIcon &displayAdapters() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("video-display"));
            return icon;
        }
        inline const QIcon &dvdCdromDrives() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-optical"));
            return icon;
        }
        inline const QIcon &hid() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("input-tablet"));
            return icon;
        }
        inline const QIcon &ideAtapiControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
            return icon;
        }
        inline const QIcon &keyboards() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("input-keyboard"));
            return icon;
        }
        inline const QIcon &mice() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("input-mouse"));
            return icon;
        }
        inline const QIcon &monitor() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("monitor"));
            return icon;
        }
        inline const QIcon &networkAdapters() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("network-wired"));
            return icon;
        }
        inline const QIcon &other() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("preferences-other"));
            return icon;
        }
        inline const QIcon &printer() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("printer"));
            return icon;
        }
        inline const QIcon &processors() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("preferences-devices-cpu"));
            return icon;
        }
        inline const QIcon &soundVideoGameControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("preferences-desktop-sound"));
            return icon;
        }
        inline const QIcon &storageControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
            return icon;
        }
        inline const QIcon &storageVolumes() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-partition"));
            return icon;
        }
        inline const QIcon &systemDevices() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("computer"));
            return icon;
        }
        inline const QIcon &usbControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-removable-media-usb"));
            return icon;
        }

        // Get icon for a subsystem (shared utility)
        inline const QIcon &forSubsystem(const QString &subsystem) {
            if (subsystem == QStringLiteral("pci"))
                return systemDevices();
            if (subsystem == QStringLiteral("usb"))
                return usbControllers();
            if (subsystem == QStringLiteral("block"))
                return diskDrives();
            if (subsystem == QStringLiteral("net"))
                return networkAdapters();
            if (subsystem == QStringLiteral("input"))
                return keyboards();
            if (subsystem == QStringLiteral("sound"))
                return audioInputs();
            if (subsystem == QStringLiteral("drm"))
                return displayAdapters();
            if (subsystem == QStringLiteral("hid"))
                return hid();
            return other();
        }
    } // namespace categoryIcons
} // namespace strings
