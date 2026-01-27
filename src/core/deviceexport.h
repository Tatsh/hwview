// SPDX-License-Identifier: MIT
/** @file */
#pragma once

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QList>
#include <QtCore/QString>

class DeviceInfo;

/**
 * @brief Exports hardware viewer data to a JSON file for viewing in a separate application.
 *
 * This class provides functionality to serialise all device information and device properties
 * to a JSON format that can be opened by a "Hardware Viewer" app. The export includes
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
     * @param devices List of devices to export.
     * @param hostname The hostname of the system being exported.
     * @returns @c true if export was successful, @c false otherwise.
     */
    static bool exportToFile(const QString &filePath,
                             const QList<DeviceInfo> &devices,
                             const QString &hostname);

    /**
     * @brief Creates a JSON object containing all export data.
     * @param devices List of devices to include in the export.
     * @param hostname The hostname of the system being exported.
     * @returns A QJsonObject containing the complete export data for all views.
     */
    static QJsonObject createExportData(const QList<DeviceInfo> &devices, const QString &hostname);

    /**
     * @brief File extension for export files.
     */
    static constexpr const auto *FILE_EXTENSION = ".dmexport";

    /**
     * @brief MIME type for export files.
     */
    static constexpr const auto *MIME_TYPE = "application/x-hwview-export";

    /**
     * @brief Current export format version.
     */
    static constexpr auto FORMAT_VERSION = 1;

    /**
     * @brief Serialises a DeviceInfo object to JSON.
     * @param info The device info to serialise.
     * @returns A QJsonObject containing all device properties.
     */
    static QJsonObject serializeDevice(const DeviceInfo &info);

    /**
     * @brief Collects system information for the export.
     * @param hostname The hostname to include.
     * @returns A QJsonObject containing system metadata.
     */
    static QJsonObject collectSystemInfo(const QString &hostname);

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
     * @brief Serialises driver information to JSON.
     * @param info The device info.
     * @returns A QJsonObject containing driver details.
     */
    static QJsonObject serializeDriverInfo(const DeviceInfo &info);
};
