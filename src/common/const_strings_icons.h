// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QString>
#include <QtGui/QIcon>

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
    static const auto icon = QIcon::fromTheme(QStringLiteral("audio-card"));
    return icon;
}

/**
 * @brief Returns the icon for battery devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &batteries() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("battery-ups"));
    return icon;
}

/**
 * @brief Returns the icon for the computer.
 * @returns Reference to the cached icon.
 */
inline const QIcon &computer() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("computer"));
    return icon;
}

/**
 * @brief Returns the icon for disk drives.
 * @returns Reference to the cached icon.
 */
inline const QIcon &diskDrives() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
    return icon;
}

/**
 * @brief Returns the icon for display adapters.
 * @returns Reference to the cached icon.
 */
inline const QIcon &displayAdapters() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("video-display"));
    return icon;
}

/**
 * @brief Returns the icon for DVD/CD-ROM drives.
 * @returns Reference to the cached icon.
 */
inline const QIcon &dvdCdromDrives() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("drive-optical"));
    return icon;
}

/**
 * @brief Returns the icon for HID devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &hid() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("input-tablet"));
    return icon;
}

/**
 * @brief Returns the icon for IDE/ATAPI controllers.
 * @returns Reference to the cached icon.
 */
inline const QIcon &ideAtapiControllers() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
    return icon;
}

/**
 * @brief Returns the icon for keyboards.
 * @returns Reference to the cached icon.
 */
inline const QIcon &keyboards() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("input-keyboard"));
    return icon;
}

/**
 * @brief Returns the icon for mice and pointing devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &mice() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("input-mouse"));
    return icon;
}

/**
 * @brief Returns the icon for monitors.
 * @returns Reference to the cached icon.
 */
inline const QIcon &monitor() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("monitor"));
    return icon;
}

/**
 * @brief Returns the icon for network adapters.
 * @returns Reference to the cached icon.
 */
inline const QIcon &networkAdapters() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("network-wired"));
    return icon;
}

/**
 * @brief Returns the icon for wireless network adapters.
 * @returns Reference to the cached icon.
 */
inline const QIcon &networkWireless() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("network-wireless"));
    return icon;
}

/**
 * @brief Returns the icon for other/unknown devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &other() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("preferences-other"));
    return icon;
}

/**
 * @brief Returns the icon for printers.
 * @returns Reference to the cached icon.
 */
inline const QIcon &printer() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("printer"));
    return icon;
}

/**
 * @brief Returns the icon for processors.
 * @returns Reference to the cached icon.
 */
inline const QIcon &processors() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("preferences-devices-cpu"));
    return icon;
}

/**
 * @brief Returns the icon for sound/video/game controllers.
 * @returns Reference to the cached icon.
 */
inline const QIcon &soundVideoGameControllers() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("preferences-desktop-sound"));
    return icon;
}

/**
 * @brief Returns the icon for storage controllers.
 * @returns Reference to the cached icon.
 */
inline const QIcon &storageControllers() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("drive-harddisk"));
    return icon;
}

/**
 * @brief Returns the icon for storage volumes.
 * @returns Reference to the cached icon.
 */
inline const QIcon &storageVolumes() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("drive-partition"));
    return icon;
}

/**
 * @brief Returns the icon for system devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &systemDevices() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("computer"));
    return icon;
}

/**
 * @brief Returns the icon for Thunderbolt devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &thunderbolt() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("thunderbolt"));
    return icon;
}

/**
 * @brief Returns the icon for USB controllers.
 * @returns Reference to the cached icon.
 */
inline const QIcon &usbControllers() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("drive-removable-media-usb"));
    return icon;
}

/**
 * @brief Returns the icon for card readers.
 * @returns Reference to the cached icon.
 */
inline const QIcon &cardReader() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("media-flash-sd-mmc"));
    return icon;
}

/**
 * @brief Returns the icon for webcam/camera devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &camera() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("camera-web"));
    return icon;
}

/**
 * @brief Returns the icon for Bluetooth devices.
 * @returns Reference to the cached icon.
 */
inline const QIcon &bluetooth() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth"));
    return icon;
}

/**
 * @brief Returns the icon for DMA resources.
 * @returns Reference to the cached icon.
 */
inline const QIcon &dma() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("memory"));
    return icon;
}

/**
 * @brief Returns the icon for I/O port resources.
 * @returns Reference to the cached icon.
 */
inline const QIcon &ioPorts() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("network-connect"));
    return icon;
}

/**
 * @brief Returns the icon for IRQ resources.
 * @returns Reference to the cached icon.
 */
inline const QIcon &irq() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("dialog-warning"));
    return icon;
}

/**
 * @brief Returns the icon for memory resources.
 * @returns Reference to the cached icon.
 */
inline const QIcon &memory() {
    static const auto icon = QIcon::fromTheme(QStringLiteral("memory"));
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

/**
 * @brief Returns an appropriate icon for a given category name.
 * @param category The category name (translated).
 * @returns Reference to the appropriate cached icon.
 */
inline const QIcon &forCategory(const QString &category) {
    if (category.contains(QStringLiteral("Sound"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("video"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("game"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("Audio"), Qt::CaseInsensitive))
        return soundVideoGameControllers();
    if (category.contains(QStringLiteral("Display"), Qt::CaseInsensitive))
        return displayAdapters();
    if (category.contains(QStringLiteral("Network"), Qt::CaseInsensitive))
        return networkAdapters();
    if (category.contains(QStringLiteral("Storage"), Qt::CaseInsensitive))
        return storageControllers();
    if (category.contains(QStringLiteral("USB"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("Serial Bus"), Qt::CaseInsensitive))
        return usbControllers();
    if (category.contains(QStringLiteral("Disk"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("DVD"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("CD-ROM"), Qt::CaseInsensitive))
        return diskDrives();
    if (category.contains(QStringLiteral("Human Interface"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("HID"), Qt::CaseInsensitive))
        return hid();
    if (category.contains(QStringLiteral("Keyboard"), Qt::CaseInsensitive))
        return keyboards();
    if (category.contains(QStringLiteral("Mice"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("Mouse"), Qt::CaseInsensitive) ||
        category.contains(QStringLiteral("pointing"), Qt::CaseInsensitive))
        return mice();
    if (category.contains(QStringLiteral("Batter"), Qt::CaseInsensitive))
        return batteries();
    if (category.contains(QStringLiteral("System"), Qt::CaseInsensitive))
        return systemDevices();
    if (category.contains(QStringLiteral("Software"), Qt::CaseInsensitive))
        return other();
    return other();
}

/**
 * @brief Returns an appropriate icon for a given driver name.
 * @param driver The driver name.
 * @returns Reference to the appropriate cached icon.
 */
inline const QIcon &forDriver(const QString &driver) {
    // Battery drivers
    if (driver == QStringLiteral("battery"))
        return batteries();

    // HID/Input drivers
    if (driver == QStringLiteral("hid-multitouch") || driver == QStringLiteral("hid-generic") ||
        driver == QStringLiteral("hid-apple") || driver == QStringLiteral("hid-logitech") ||
        driver == QStringLiteral("hid-logitech-hidpp") || driver == QStringLiteral("usbhid") ||
        driver == QStringLiteral("i2c_hid_acpi"))
        return hid();

    // Keyboard drivers
    if (driver == QStringLiteral("atkbd") || driver == QStringLiteral("i8042"))
        return keyboards();

    // Mouse/touchpad drivers
    if (driver == QStringLiteral("psmouse") || driver == QStringLiteral("elan_i2c") ||
        driver == QStringLiteral("synaptics"))
        return mice();

    // Graphics drivers
    if (driver == QStringLiteral("i915") || driver == QStringLiteral("nvidia") ||
        driver == QStringLiteral("nvidia-gpu") || driver == QStringLiteral("nouveau") ||
        driver == QStringLiteral("amdgpu") || driver == QStringLiteral("radeon"))
        return displayAdapters();

    // Network drivers (wired)
    if (driver == QStringLiteral("r8169") || driver == QStringLiteral("e1000e") ||
        driver == QStringLiteral("igb") || driver == QStringLiteral("igc"))
        return networkAdapters();

    // Network drivers (wireless)
    if (driver == QStringLiteral("iwlwifi") || driver == QStringLiteral("ath9k") ||
        driver == QStringLiteral("ath10k_pci") || driver == QStringLiteral("ath11k_pci") ||
        driver == QStringLiteral("rtw88_pci") || driver == QStringLiteral("rtw89_pci") ||
        driver == QStringLiteral("mt7921e") || driver == QStringLiteral("brcmfmac"))
        return networkWireless();

    // Storage drivers
    if (driver == QStringLiteral("nvme") || driver == QStringLiteral("ahci") ||
        driver == QStringLiteral("sd") || driver == QStringLiteral("uas") ||
        driver == QStringLiteral("usb-storage"))
        return diskDrives();

    // Card reader drivers
    if (driver == QStringLiteral("rtsx_pci") || driver == QStringLiteral("sdhci-pci"))
        return cardReader();

    // Audio drivers
    if (driver == QStringLiteral("snd_hda_intel") ||
        driver == QStringLiteral("snd_sof_pci_intel_tgl") ||
        driver == QStringLiteral("sof-audio-pci-intel-tgl") ||
        driver == QStringLiteral("snd_hda_codec_realtek") ||
        driver == QStringLiteral("snd_hda_codec_hdmi") || driver == QStringLiteral("snd_usb_audio"))
        return audioInputs();

    // Video/camera drivers
    if (driver == QStringLiteral("uvcvideo"))
        return camera();

    // Bluetooth drivers
    if (driver == QStringLiteral("btusb") || driver == QStringLiteral("btintel") ||
        driver == QStringLiteral("btrtl") || driver == QStringLiteral("btmtk"))
        return bluetooth();

    // USB host controllers
    if (driver == QStringLiteral("xhci_hcd") || driver == QStringLiteral("ehci-pci") ||
        driver == QStringLiteral("ohci-pci") || driver == QStringLiteral("uhci_hcd"))
        return usbControllers();

    // Thunderbolt
    if (driver == QStringLiteral("thunderbolt"))
        return usbControllers();

    // Processor/thermal drivers
    if (driver == QStringLiteral("proc_thermal") || driver == QStringLiteral("processor") ||
        driver == QStringLiteral("acpi-cpufreq") || driver == QStringLiteral("intel_pstate"))
        return processors();

    // System/platform drivers (SMBus, SPI, LPSS, MEI, etc.)
    if (driver == QStringLiteral("i801_smbus") || driver == QStringLiteral("i2c_designware") ||
        driver == QStringLiteral("intel-lpss") || driver == QStringLiteral("intel-spi") ||
        driver == QStringLiteral("intel-vsec") || driver == QStringLiteral("mei_me") ||
        driver == QStringLiteral("pcieport") || driver == QStringLiteral("pci-stub") ||
        driver == QStringLiteral("portdrv") || driver == QStringLiteral("shpchp") ||
        driver == QStringLiteral("serial8250") || driver == QStringLiteral("dw-apb-uart") ||
        driver == QStringLiteral("hub") || driver == QStringLiteral("rtc_cmos"))
        return systemDevices();

    return other();
}

} // namespace categoryIcons
} // namespace strings
