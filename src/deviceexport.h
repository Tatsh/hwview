#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class DeviceInfo;

/**
 * @brief Exports device manager data to a JSON file for viewing in a separate application.
 *
 * This class provides functionality to serialize all device information and device properties
 * to a JSON format that can be opened by a "Device Manager Viewer" app. The export includes
 * all data necessary to display any view (Devices by Type, Devices by Connection, etc.) and
 * always includes hidden devices regardless of the current display setting.
 *
 * This is useful for debugging system issues by capturing a complete snapshot of the device state.
 */
class DeviceExport {
public:
    /**
     * @brief Exports all device data to a JSON file.
     *
     * Exports complete device information including all properties needed to display any view
     * and all hidden devices. The viewer application can reconstruct any view from this data.
     *
     * @param filePath The path to save the export file.
     * @returns @c true if export was successful, @c false otherwise.
     */
    static bool exportToFile(const QString &filePath);

    /**
     * @brief Creates a JSON object containing all export data.
     * @returns A QJsonObject containing the complete export data for all views.
     */
    static QJsonObject createExportData();

    /**
     * @brief File extension for export files.
     */
    static constexpr const char *FILE_EXTENSION = ".dmexport";

    /**
     * @brief MIME type for export files.
     */
    static constexpr const char *MIME_TYPE = "application/x-devmgmt-export";

    /**
     * @brief Current export format version.
     */
    static constexpr int FORMAT_VERSION = 1;

private:
    /**
     * @brief Serializes a DeviceInfo object to JSON.
     * @param info The device info to serialize.
     * @returns A QJsonObject containing all device properties.
     */
    static QJsonObject serializeDevice(const DeviceInfo &info);

    /**
     * @brief Collects system information for the export.
     * @returns A QJsonObject containing system metadata.
     */
    static QJsonObject collectSystemInfo();

    /**
     * @brief Collects system-wide resources for Resources views.
     *
     * On Linux, this includes content from /proc/dma, /proc/ioports,
     * /proc/interrupts, and /proc/iomem.
     *
     * @returns A QJsonObject containing system resource data.
     */
    static QJsonObject collectSystemResources();

    /**
     * @brief Gets device resources for a device (IRQ, memory, I/O).
     * @param syspath The device system path.
     * @returns A QJsonArray containing resource information.
     */
    static QJsonArray getDeviceResources(const QString &syspath);

    /**
     * @brief Gets device driver information.
     * @param info The device info.
     * @returns A QJsonObject containing driver details.
     */
    static QJsonObject getDriverInfo(const DeviceInfo &info);
};
