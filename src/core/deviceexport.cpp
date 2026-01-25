#include "deviceexport.h"
#include "deviceinfo.h"
#include "systeminfo.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QLocale>
#include <QSysInfo>

bool DeviceExport::exportToFile(const QString &filePath,
                                const QList<DeviceInfo> &devices,
                                const QString &hostname) {
    QJsonObject exportData = createExportData(devices, hostname);
    QJsonDocument doc(exportData);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QJsonObject DeviceExport::createExportData(const QList<DeviceInfo> &devices,
                                           const QString &hostname) {
    QJsonObject root;

    // Format metadata
    root[QStringLiteral("formatVersion")] = FORMAT_VERSION;
    root[QStringLiteral("mimeType")] = QLatin1String(MIME_TYPE);
    root[QStringLiteral("exportDate")] =
        QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    root[QStringLiteral("applicationName")] = QCoreApplication::applicationName();
    root[QStringLiteral("applicationVersion")] = QCoreApplication::applicationVersion();

    // System information
    root[QStringLiteral("system")] = collectSystemInfo(hostname);

    // Note: Hidden devices are always included in the export
    root[QStringLiteral("includesHiddenDevices")] = true;

    // All devices with full properties (including hidden devices)
    // The viewer can reconstruct any view from this complete device list
    QJsonArray devicesArray;
    for (const DeviceInfo &info : devices) {
        devicesArray.append(serializeDevice(info));
    }
    root[QStringLiteral("devices")] = devicesArray;

    // System resources for Resources views (Linux only)
    root[QStringLiteral("systemResources")] = collectSystemResources();

    return root;
}

QJsonObject DeviceExport::serializeDevice(const DeviceInfo &info) {
    QJsonObject device;

    // Basic device identification
    device[QStringLiteral("syspath")] = info.syspath();
    device[QStringLiteral("name")] = info.name();
    device[QStringLiteral("driver")] = info.driver();
    device[QStringLiteral("subsystem")] = info.subsystem();
    device[QStringLiteral("devnode")] = info.devnode();
    device[QStringLiteral("parentSyspath")] = info.parentSyspath();
    device[QStringLiteral("devPath")] = info.devPath();
    device[QStringLiteral("isHidden")] = info.isHidden();
    device[QStringLiteral("isValidForDisplay")] = info.isValidForDisplay();

    // Category
    device[QStringLiteral("category")] = static_cast<int>(info.category());

    // Category name for human readability
    QString categoryName;
    switch (info.category()) {
    case DeviceCategory::AudioInputsAndOutputs:
        categoryName = QStringLiteral("Audio inputs and outputs");
        break;
    case DeviceCategory::Batteries:
        categoryName = QStringLiteral("Batteries");
        break;
    case DeviceCategory::Computer:
        categoryName = QStringLiteral("Computer");
        break;
    case DeviceCategory::DiskDrives:
        categoryName = QStringLiteral("Disk drives");
        break;
    case DeviceCategory::DisplayAdapters:
        categoryName = QStringLiteral("Display adapters");
        break;
    case DeviceCategory::DvdCdromDrives:
        categoryName = QStringLiteral("DVD/CD-ROM drives");
        break;
    case DeviceCategory::HumanInterfaceDevices:
        categoryName = QStringLiteral("Human Interface Devices");
        break;
    case DeviceCategory::Keyboards:
        categoryName = QStringLiteral("Keyboards");
        break;
    case DeviceCategory::MiceAndOtherPointingDevices:
        categoryName = QStringLiteral("Mice and other pointing devices");
        break;
    case DeviceCategory::NetworkAdapters:
        categoryName = QStringLiteral("Network adapters");
        break;
    case DeviceCategory::SoftwareDevices:
        categoryName = QStringLiteral("Software devices");
        break;
    case DeviceCategory::SoundVideoAndGameControllers:
        categoryName = QStringLiteral("Sound, video and game controllers");
        break;
    case DeviceCategory::StorageControllers:
        categoryName = QStringLiteral("Storage controllers");
        break;
    case DeviceCategory::StorageVolumes:
        categoryName = QStringLiteral("Storage volumes");
        break;
    case DeviceCategory::SystemDevices:
        categoryName = QStringLiteral("System devices");
        break;
    case DeviceCategory::UniversalSerialBusControllers:
        categoryName = QStringLiteral("Universal Serial Bus controllers");
        break;
    case DeviceCategory::Unknown:
    default:
        categoryName = QStringLiteral("Unknown");
        break;
    }
    device[QStringLiteral("categoryName")] = categoryName;

    // PCI information
    if (!info.pciClass().isEmpty()) {
        QJsonObject pci;
        pci[QStringLiteral("class")] = info.pciClass();
        pci[QStringLiteral("subclass")] = info.pciSubclass();
        pci[QStringLiteral("interface")] = info.pciInterface();
        device[QStringLiteral("pci")] = pci;
    }

    // ID properties
    QJsonObject ids;
    ids[QStringLiteral("cdrom")] = info.idCdrom();
    ids[QStringLiteral("devType")] = info.devType();
    ids[QStringLiteral("inputKeyboard")] = info.idInputKeyboard();
    ids[QStringLiteral("inputMouse")] = info.idInputMouse();
    ids[QStringLiteral("type")] = info.idType();
    ids[QStringLiteral("modelFromDatabase")] = info.idModelFromDatabase();
    device[QStringLiteral("ids")] = ids;

    // Fetch additional platform-specific properties
    QHash<QString, QString> exportProps = getExportDeviceProperties(info);
    if (!exportProps.isEmpty()) {
        QJsonObject properties;
        for (auto it = exportProps.begin(); it != exportProps.end(); ++it) {
            properties[it.key()] = it.value();
        }
        device[QStringLiteral("properties")] = properties;
    }

    // Driver information
    device[QStringLiteral("driverInfo")] = serializeDriverInfo(info);

    // Resources (for PCI devices)
    QList<ExportResourceInfo> resList = getExportDeviceResources(info.syspath());
    if (!resList.isEmpty()) {
        QJsonArray resources;
        for (const auto &res : resList) {
            QJsonObject resObj;
            resObj[QStringLiteral("type")] = res.type;
            resObj[QStringLiteral("displayValue")] = res.displayValue;
            if (!res.start.isEmpty()) {
                resObj[QStringLiteral("start")] = res.start;
            }
            if (!res.end.isEmpty()) {
                resObj[QStringLiteral("end")] = res.end;
            }
            if (!res.flags.isEmpty()) {
                resObj[QStringLiteral("flags")] = res.flags;
            }
            if (res.value != 0) {
                resObj[QStringLiteral("value")] = res.value;
            }
            resources.append(resObj);
        }
        device[QStringLiteral("resources")] = resources;
    }

    return device;
}

QJsonObject DeviceExport::collectSystemInfo(const QString &hostname) {
    QJsonObject info;

    info[QStringLiteral("hostname")] = hostname;
    info[QStringLiteral("productType")] = QSysInfo::productType();
    info[QStringLiteral("productVersion")] = QSysInfo::productVersion();
    info[QStringLiteral("prettyProductName")] = QSysInfo::prettyProductName();
    info[QStringLiteral("kernelType")] = QSysInfo::kernelType();
    info[QStringLiteral("kernelVersion")] = QSysInfo::kernelVersion();
    info[QStringLiteral("cpuArchitecture")] = QSysInfo::currentCpuArchitecture();
    info[QStringLiteral("buildCpuArchitecture")] = QSysInfo::buildCpuArchitecture();
    info[QStringLiteral("locale")] = QLocale::system().name();

    UnameInfo unameInfo = getUnameInfo();
    if (unameInfo.valid) {
        info[QStringLiteral("unameSysname")] = unameInfo.sysname;
        info[QStringLiteral("unameRelease")] = unameInfo.release;
        info[QStringLiteral("unameVersion")] = unameInfo.version;
        info[QStringLiteral("unameMachine")] = unameInfo.machine;
    }

    QHash<QString, QString> distro = getDistributionInfo();
    if (!distro.isEmpty()) {
        QJsonObject distroObj;
        for (auto it = distro.begin(); it != distro.end(); ++it) {
            distroObj[it.key()] = it.value();
        }
        info[QStringLiteral("distribution")] = distroObj;
    }

    return info;
}

QJsonObject DeviceExport::collectSystemResources() {
    QJsonObject resources;

    QHash<QString, QString> rawResources = getSystemResourcesRaw();
    for (auto it = rawResources.begin(); it != rawResources.end(); ++it) {
        resources[it.key()] = it.value();
    }

    return resources;
}

QJsonObject DeviceExport::serializeDriverInfo(const DeviceInfo &info) {
    QJsonObject driverInfoObj;

    ExportDriverInfo driverInfo = getExportDriverInfo(info);

    driverInfoObj[QStringLiteral("hasDriver")] = driverInfo.hasDriver;
    if (!driverInfo.hasDriver) {
        return driverInfoObj;
    }

    driverInfoObj[QStringLiteral("name")] = driverInfo.name;

    auto addIfNotEmpty = [&](const QString &key, const QString &value) {
        if (!value.isEmpty()) {
            driverInfoObj[key] = value;
        }
    };

    addIfNotEmpty(QStringLiteral("filename"), driverInfo.filename);
    addIfNotEmpty(QStringLiteral("author"), driverInfo.author);
    addIfNotEmpty(QStringLiteral("version"), driverInfo.version);
    addIfNotEmpty(QStringLiteral("license"), driverInfo.license);
    addIfNotEmpty(QStringLiteral("description"), driverInfo.description);
    addIfNotEmpty(QStringLiteral("signer"), driverInfo.signer);
    addIfNotEmpty(QStringLiteral("srcversion"), driverInfo.srcversion);
    addIfNotEmpty(QStringLiteral("vermagic"), driverInfo.vermagic);
    addIfNotEmpty(QStringLiteral("date"), driverInfo.date);
    addIfNotEmpty(QStringLiteral("bundleIdentifier"), driverInfo.bundleIdentifier);
    addIfNotEmpty(QStringLiteral("provider"), driverInfo.provider);

    if (driverInfo.isOutOfTree) {
        driverInfoObj[QStringLiteral("isOutOfTree")] = true;
    }
    if (driverInfo.isBuiltin) {
        driverInfoObj[QStringLiteral("isBuiltin")] = true;
    }

    return driverInfoObj;
}
