#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

/**
 * @brief Holds device data loaded from an exported JSON file.
 *
 * This class provides an interface similar to DeviceCache but works with
 * data loaded from a .dmexport file instead of querying live system data.
 */
class ExportedData : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Represents a device loaded from the export.
     */
    struct Device {
        QString syspath;
        QString name;
        QString driver;
        QString subsystem;
        QString devnode;
        QString parentSyspath;
        QString devPath;
        QString categoryName;
        int category = 0;
        bool isHidden = false;
        bool isValidForDisplay = false;

        // PCI info
        QString pciClass;
        QString pciSubclass;
        QString pciInterface;

        // ID properties
        QString idCdrom;
        QString idDevType;
        QString idInputKeyboard;
        QString idInputMouse;
        QString idType;
        QString idModelFromDatabase;

        // Additional properties
        QJsonObject properties;
        QJsonObject driverInfo;
        QJsonArray resources;
    };

    /**
     * @brief Returns the singleton instance.
     * @returns Reference to the global ExportedData instance.
     */
    static ExportedData &instance();

    /**
     * @brief Loads data from an export file.
     * @param filePath Path to the .dmexport file.
     * @returns @c true if loaded successfully, @c false otherwise.
     */
    bool loadFromFile(const QString &filePath);

    /**
     * @brief Returns whether data has been loaded.
     * @returns @c true if data is loaded, @c false otherwise.
     */
    bool isLoaded() const;

    /**
     * @brief Returns the hostname from the export.
     * @returns The hostname string.
     */
    const QString &hostname() const;

    /**
     * @brief Returns all loaded devices.
     * @returns List of all devices.
     */
    const QList<Device> &allDevices() const;

    /**
     * @brief Finds a device by its syspath.
     * @param syspath The system path to search for.
     * @returns Pointer to the device if found, or @c nullptr.
     */
    const Device *deviceBySyspath(const QString &syspath) const;

    /**
     * @brief Returns system information from the export.
     * @returns The system info JSON object.
     */
    const QJsonObject &systemInfo() const;

    /**
     * @brief Returns system resources (for Resources views).
     * @returns The system resources JSON object.
     */
    const QJsonObject &systemResources() const;

    /**
     * @brief Returns the export date.
     * @returns The export date string.
     */
    const QString &exportDate() const;

    /**
     * @brief Returns the source application name.
     * @returns The application name from the export.
     */
    const QString &sourceApplicationName() const;

    /**
     * @brief Returns the source application version.
     * @returns The application version from the export.
     */
    const QString &sourceApplicationVersion() const;

    /**
     * @brief Returns the path of the currently loaded file.
     * @returns The file path, or empty string if not loaded.
     */
    const QString &currentFilePath() const;

    /**
     * @brief Returns whether to show hidden devices.
     * @returns @c true if hidden devices should be shown.
     */
    bool showHiddenDevices() const;

    /**
     * @brief Sets whether to show hidden devices.
     * @param show @c true to show hidden devices.
     */
    void setShowHiddenDevices(bool show);

Q_SIGNALS:
    /**
     * @brief Emitted when data is loaded from a file.
     */
    void dataLoaded();

private:
    ExportedData();
    ~ExportedData() override = default;
    ExportedData(const ExportedData &) = delete;
    ExportedData &operator=(const ExportedData &) = delete;

    void clear();
    Device parseDevice(const QJsonObject &obj);

    bool loaded_ = false;
    bool showHiddenDevices_ = true;
    QString filePath_;
    QString hostname_;
    QString exportDate_;
    QString sourceAppName_;
    QString sourceAppVersion_;
    QList<Device> devices_;
    QHash<QString, int> syspathIndex_;
    QJsonObject systemInfo_;
    QJsonObject systemResources_;
};
