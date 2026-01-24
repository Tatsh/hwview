#pragma once

#include <QIcon>
#include <QString>

namespace strings {

    /**
     * @brief Namespace containing cached category icons.
     *
     * These functions provide cached QIcon objects for device categories, avoiding repeated theme
     * lookups for better performance.
     */
    namespace categoryIcons {

        /**
         * @brief Returns the icon for audio input/output devices.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &audioInputs() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("audio-card"));
            return icon;
        }

        /**
         * @brief Returns the icon for battery devices.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &batteries() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("battery-ups"));
            return icon;
        }

        /**
         * @brief Returns the icon for the computer.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &computer() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("computer"));
            return icon;
        }

        /**
         * @brief Returns the icon for disk drives.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &diskDrives() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
            return icon;
        }

        /**
         * @brief Returns the icon for display adapters.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &displayAdapters() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("video-display"));
            return icon;
        }

        /**
         * @brief Returns the icon for DVD/CD-ROM drives.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &dvdCdromDrives() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-optical"));
            return icon;
        }

        /**
         * @brief Returns the icon for HID devices.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &hid() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("input-tablet"));
            return icon;
        }

        /**
         * @brief Returns the icon for IDE/ATAPI controllers.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &ideAtapiControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
            return icon;
        }

        /**
         * @brief Returns the icon for keyboards.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &keyboards() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("input-keyboard"));
            return icon;
        }

        /**
         * @brief Returns the icon for mice and pointing devices.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &mice() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("input-mouse"));
            return icon;
        }

        /**
         * @brief Returns the icon for monitors.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &monitor() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("monitor"));
            return icon;
        }

        /**
         * @brief Returns the icon for network adapters.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &networkAdapters() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("network-wired"));
            return icon;
        }

        /**
         * @brief Returns the icon for other/unknown devices.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &other() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("preferences-other"));
            return icon;
        }

        /**
         * @brief Returns the icon for printers.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &printer() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("printer"));
            return icon;
        }

        /**
         * @brief Returns the icon for processors.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &processors() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("preferences-devices-cpu"));
            return icon;
        }

        /**
         * @brief Returns the icon for sound/video/game controllers.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &soundVideoGameControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("preferences-desktop-sound"));
            return icon;
        }

        /**
         * @brief Returns the icon for storage controllers.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &storageControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
            return icon;
        }

        /**
         * @brief Returns the icon for storage volumes.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &storageVolumes() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-partition"));
            return icon;
        }

        /**
         * @brief Returns the icon for system devices.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &systemDevices() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("computer"));
            return icon;
        }

        /**
         * @brief Returns the icon for USB controllers.
         * @returns Reference to the cached icon.
         */
        inline const QIcon &usbControllers() {
            static const QIcon icon = QIcon::fromTheme(QStringLiteral("drive-removable-media-usb"));
            return icon;
        }

        /**
         * @brief Returns an appropriate icon for a given subsystem.
         * @param subsystem The device subsystem name.
         * @returns Reference to the appropriate cached icon.
         */
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
