#pragma once

#include <devguid.h>
#include <initguid.h>
#include <windows.h>

#include "deviceinfo.h"

// Device class GUID to DeviceCategory mapping
// These GUIDs are defined in devguid.h from the Windows SDK

namespace setupapi {

// Map a device class GUID to our DeviceCategory enum
inline DeviceCategory classGuidToCategory(const GUID &guid) {
    // USB Controllers
    if (IsEqualGUID(guid, GUID_DEVCLASS_USB)) {
        return DeviceCategory::UniversalSerialBusControllers;
    }

    // Disk Drives
    if (IsEqualGUID(guid, GUID_DEVCLASS_DISKDRIVE)) {
        return DeviceCategory::DiskDrives;
    }

    // Display Adapters
    if (IsEqualGUID(guid, GUID_DEVCLASS_DISPLAY)) {
        return DeviceCategory::DisplayAdapters;
    }

    // Network Adapters
    if (IsEqualGUID(guid, GUID_DEVCLASS_NET)) {
        return DeviceCategory::NetworkAdapters;
    }

    // Keyboards
    if (IsEqualGUID(guid, GUID_DEVCLASS_KEYBOARD)) {
        return DeviceCategory::Keyboards;
    }

    // Mice and pointing devices
    if (IsEqualGUID(guid, GUID_DEVCLASS_MOUSE)) {
        return DeviceCategory::MiceAndOtherPointingDevices;
    }

    // Sound, video and game controllers (Media)
    if (IsEqualGUID(guid, GUID_DEVCLASS_MEDIA)) {
        return DeviceCategory::SoundVideoAndGameControllers;
    }

    // Human Interface Devices
    if (IsEqualGUID(guid, GUID_DEVCLASS_HIDCLASS)) {
        return DeviceCategory::HumanInterfaceDevices;
    }

    // Storage Volumes
    if (IsEqualGUID(guid, GUID_DEVCLASS_VOLUME)) {
        return DeviceCategory::StorageVolumes;
    }

    // Storage Controllers (SCSI adapters)
    if (IsEqualGUID(guid, GUID_DEVCLASS_SCSIADAPTER)) {
        return DeviceCategory::StorageControllers;
    }

    // Also handle HDC (IDE controllers) as storage controllers
    if (IsEqualGUID(guid, GUID_DEVCLASS_HDC)) {
        return DeviceCategory::StorageControllers;
    }

    // Batteries
    if (IsEqualGUID(guid, GUID_DEVCLASS_BATTERY)) {
        return DeviceCategory::Batteries;
    }

    // System devices
    if (IsEqualGUID(guid, GUID_DEVCLASS_SYSTEM)) {
        return DeviceCategory::SystemDevices;
    }

    // CD-ROM drives
    if (IsEqualGUID(guid, GUID_DEVCLASS_CDROM)) {
        return DeviceCategory::DvdCdromDrives;
    }

    // Audio devices (separate from media)
    if (IsEqualGUID(guid, GUID_DEVCLASS_AUDIOENDPOINT)) {
        return DeviceCategory::AudioInputsAndOutputs;
    }

    // Processors
    if (IsEqualGUID(guid, GUID_DEVCLASS_PROCESSOR)) {
        return DeviceCategory::SystemDevices;
    }

    // Monitor
    if (IsEqualGUID(guid, GUID_DEVCLASS_MONITOR)) {
        return DeviceCategory::DisplayAdapters;
    }

    // Floppy disk controllers (legacy)
    if (IsEqualGUID(guid, GUID_DEVCLASS_FDC)) {
        return DeviceCategory::StorageControllers;
    }

    // Floppy drives (legacy)
    if (IsEqualGUID(guid, GUID_DEVCLASS_FLOPPYDISK)) {
        return DeviceCategory::DiskDrives;
    }

    // Printers
    if (IsEqualGUID(guid, GUID_DEVCLASS_PRINTER)) {
        return DeviceCategory::SoftwareDevices;
    }

    // Ports (COM & LPT)
    if (IsEqualGUID(guid, GUID_DEVCLASS_PORTS)) {
        return DeviceCategory::SystemDevices;
    }

    // Modems
    if (IsEqualGUID(guid, GUID_DEVCLASS_MODEM)) {
        return DeviceCategory::NetworkAdapters;
    }

    // Bluetooth
    if (IsEqualGUID(guid, GUID_DEVCLASS_BLUETOOTH)) {
        return DeviceCategory::NetworkAdapters;
    }

    // Image devices (scanners, cameras)
    if (IsEqualGUID(guid, GUID_DEVCLASS_IMAGE)) {
        return DeviceCategory::SoundVideoAndGameControllers;
    }

    // Software devices
    if (IsEqualGUID(guid, GUID_DEVCLASS_SOFTWAREDEVICE)) {
        return DeviceCategory::SoftwareDevices;
    }

    return DeviceCategory::Unknown;
}

/**
     * @brief Converts a GUID to its string representation.
     * @param guid The GUID to convert.
     * @returns The GUID as a lowercase string with braces (e.g., "{4d36e96b-e325-11ce-bfc1-08002be10318}").
     */
inline QString guidToString(const GUID &guid) {
    wchar_t buffer[39];
    StringFromGUID2(guid, buffer, 39);
    return QString::fromWCharArray(buffer).toLower();
}

} // namespace setupapi
