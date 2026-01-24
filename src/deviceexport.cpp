#include "deviceexport.h"
#include "const_strings.h"
#include "devicecache.h"
#include "deviceinfo.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QHostInfo>
#include <QJsonDocument>
#include <QLocale>
#include <QProcess>
#include <QSysInfo>

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace {
    QString safeReadSysfsFile(const QString &path) {
        QByteArray pathBytes = path.toLocal8Bit();
        int fd = open(pathBytes.constData(), O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            return {};
        }
        char buffer[4096];
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);
        if (bytesRead <= 0) {
            return {};
        }
        buffer[bytesRead] = '\0';
        return QString::fromUtf8(buffer).trimmed();
    }

    bool isPciDevice(const QString &syspath) {
        static const QRegularExpression pciPathRe(QStringLiteral(
            "/pci[^/]*/[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\\.[0-9a-fA-F]$"));
        return pciPathRe.match(syspath).hasMatch();
    }
} // namespace
#elif defined(Q_OS_MACOS)
#include <sys/utsname.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace props = strings::udev::propertyNames;

bool DeviceExport::exportToFile(const QString &filePath) {
    QJsonObject exportData = createExportData();
    QJsonDocument doc(exportData);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QJsonObject DeviceExport::createExportData() {
    QJsonObject root;

    // Format metadata
    root[QStringLiteral("formatVersion")] = FORMAT_VERSION;
    root[QStringLiteral("mimeType")] = QLatin1String(MIME_TYPE);
    root[QStringLiteral("exportDate")] =
        QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    root[QStringLiteral("applicationName")] = QCoreApplication::applicationName();
    root[QStringLiteral("applicationVersion")] = QCoreApplication::applicationVersion();

    // System information
    root[QStringLiteral("system")] = collectSystemInfo();

    // Note: Hidden devices are always included in the export
    root[QStringLiteral("includesHiddenDevices")] = true;

    // All devices with full properties (including hidden devices)
    // The viewer can reconstruct any view from this complete device list
    QJsonArray devicesArray;
    for (const DeviceInfo &info : DeviceCache::instance().allDevices()) {
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

    // Fetch additional properties that are used in the properties dialog
#ifdef Q_OS_LINUX
    QJsonObject properties;
    if (!info.syspath().isEmpty()) {
        properties[QStringLiteral("ID_VENDOR_FROM_DATABASE")] =
            info.propertyValue(props::ID_VENDOR_FROM_DATABASE);
        properties[QStringLiteral("ID_VENDOR")] = info.propertyValue(props::ID_VENDOR);
        properties[QStringLiteral("ID_VENDOR_ENC")] = info.propertyValue(props::ID_VENDOR_ENC);
        properties[QStringLiteral("ID_USB_VENDOR")] = info.propertyValue(props::ID_USB_VENDOR);
        properties[QStringLiteral("ID_MODEL")] = info.propertyValue(props::ID_MODEL);
        properties[QStringLiteral("ID_MODEL_FROM_DATABASE")] =
            info.propertyValue(props::ID_MODEL_FROM_DATABASE);
        properties[QStringLiteral("ID_SERIAL")] = info.propertyValue("ID_SERIAL");
        properties[QStringLiteral("MODALIAS")] = info.propertyValue(props::MODALIAS);
        properties[QStringLiteral("DEVTYPE")] = info.propertyValue(props::DEVTYPE);
        properties[QStringLiteral("ID_PART_ENTRY_NAME")] =
            info.propertyValue(props::ID_PART_ENTRY_NAME);
        properties[QStringLiteral("ID_FS_LABEL")] = info.propertyValue(props::ID_FS_LABEL);
        properties[QStringLiteral("ID_VENDOR_ID")] = info.propertyValue("ID_VENDOR_ID");
        properties[QStringLiteral("ID_MODEL_ID")] = info.propertyValue("ID_MODEL_ID");

        // Remove empty properties
        QStringList keysToRemove;
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            if (it.value().toString().isEmpty()) {
                keysToRemove.append(it.key());
            }
        }
        for (const QString &key : keysToRemove) {
            properties.remove(key);
        }
    }
    if (!properties.isEmpty()) {
        device[QStringLiteral("properties")] = properties;
    }
#endif

    // Driver information
    device[QStringLiteral("driverInfo")] = getDriverInfo(info);

    // Resources (for PCI devices on Linux)
    QJsonArray resources = getDeviceResources(info.syspath());
    if (!resources.isEmpty()) {
        device[QStringLiteral("resources")] = resources;
    }

    return device;
}

QJsonObject DeviceExport::collectSystemInfo() {
    QJsonObject info;

    info[QStringLiteral("hostname")] = QHostInfo::localHostName();
    info[QStringLiteral("productType")] = QSysInfo::productType();
    info[QStringLiteral("productVersion")] = QSysInfo::productVersion();
    info[QStringLiteral("prettyProductName")] = QSysInfo::prettyProductName();
    info[QStringLiteral("kernelType")] = QSysInfo::kernelType();
    info[QStringLiteral("kernelVersion")] = QSysInfo::kernelVersion();
    info[QStringLiteral("cpuArchitecture")] = QSysInfo::currentCpuArchitecture();
    info[QStringLiteral("buildCpuArchitecture")] = QSysInfo::buildCpuArchitecture();
    info[QStringLiteral("locale")] = QLocale::system().name();

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        info[QStringLiteral("unameSysname")] = QString::fromLocal8Bit(buffer.sysname);
        info[QStringLiteral("unameRelease")] = QString::fromLocal8Bit(buffer.release);
        info[QStringLiteral("unameVersion")] = QString::fromLocal8Bit(buffer.version);
        info[QStringLiteral("unameMachine")] = QString::fromLocal8Bit(buffer.machine);
    }
#endif

#ifdef Q_OS_LINUX
    // Read /etc/os-release for distribution info
    QFile osRelease(QStringLiteral("/etc/os-release"));
    if (osRelease.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonObject distro;
        while (!osRelease.atEnd()) {
            QString line = QString::fromUtf8(osRelease.readLine()).trimmed();
            auto eqPos = line.indexOf(QLatin1Char('='));
            if (eqPos > 0) {
                QString key = line.left(eqPos);
                QString value = line.mid(eqPos + 1);
                // Remove quotes
                if (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))) {
                    value = value.mid(1, value.length() - 2);
                }
                distro[key] = value;
            }
        }
        osRelease.close();
        info[QStringLiteral("distribution")] = distro;
    }
#endif

    return info;
}

QJsonObject DeviceExport::collectSystemResources() {
    QJsonObject resources;

#ifdef Q_OS_LINUX
    // Read /proc/dma - DMA channels
    QFile dmaFile(QStringLiteral("/proc/dma"));
    if (dmaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonArray dmaArray;
        QString content = QString::fromLocal8Bit(dmaFile.readAll());
        for (const QString &line : content.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            dmaArray.append(line);
        }
        dmaFile.close();
        resources[QStringLiteral("dma")] = dmaArray;
    }

    // Read /proc/ioports - I/O port allocations
    QFile ioportsFile(QStringLiteral("/proc/ioports"));
    if (ioportsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonArray ioportsArray;
        QString content = QString::fromLocal8Bit(ioportsFile.readAll());
        for (const QString &line : content.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            ioportsArray.append(line);
        }
        ioportsFile.close();
        resources[QStringLiteral("ioports")] = ioportsArray;
    }

    // Read /proc/interrupts - IRQ allocations
    QFile interruptsFile(QStringLiteral("/proc/interrupts"));
    if (interruptsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonArray interruptsArray;
        QString content = QString::fromLocal8Bit(interruptsFile.readAll());
        for (const QString &line : content.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            interruptsArray.append(line);
        }
        interruptsFile.close();
        resources[QStringLiteral("interrupts")] = interruptsArray;
    }

    // Read /proc/iomem - Memory-mapped I/O allocations
    QFile iomemFile(QStringLiteral("/proc/iomem"));
    if (iomemFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonArray iomemArray;
        QString content = QString::fromLocal8Bit(iomemFile.readAll());
        for (const QString &line : content.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            iomemArray.append(line);
        }
        iomemFile.close();
        resources[QStringLiteral("iomem")] = iomemArray;
    }
#endif

    return resources;
}

QJsonArray DeviceExport::getDeviceResources(const QString &syspath) {
    QJsonArray resources;

#ifdef Q_OS_LINUX
    if (syspath.isEmpty() || !isPciDevice(syspath)) {
        return resources;
    }

    // Get IRQ
    QString irq = safeReadSysfsFile(syspath + QStringLiteral("/irq"));
    if (!irq.isEmpty() && irq != QStringLiteral("0")) {
        QJsonObject irqRes;
        irqRes[QStringLiteral("type")] = QStringLiteral("IRQ");
        irqRes[QStringLiteral("value")] = irq.toInt();
        irqRes[QStringLiteral("displayValue")] = QStringLiteral("0x%1 (%2)")
                                                     .arg(irq.toInt(), 8, 16, QLatin1Char('0'))
                                                     .arg(irq)
                                                     .toUpper();
        resources.append(irqRes);
    }

    // Get PCI resources (memory ranges and I/O ports)
    QString resourceContent = safeReadSysfsFile(syspath + QStringLiteral("/resource"));
    if (!resourceContent.isEmpty()) {
        static const QRegularExpression whitespaceRe(QStringLiteral("\\s+"));
        const QStringList lines = resourceContent.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            QStringList parts = line.trimmed().split(whitespaceRe, Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                bool ok1, ok2, ok3;
                qulonglong start = parts[0].toULongLong(&ok1, 16);
                qulonglong end = parts[1].toULongLong(&ok2, 16);
                qulonglong flags = parts[2].toULongLong(&ok3, 16);

                if (ok1 && ok2 && ok3 && start != 0 && end != 0) {
                    QJsonObject res;

                    // IORESOURCE_IO = 0x00000100, IORESOURCE_MEM = 0x00000200
                    if (flags & 0x00000100) {
                        res[QStringLiteral("type")] = QStringLiteral("I/O Range");
                    } else if (flags & 0x00000200) {
                        res[QStringLiteral("type")] = QStringLiteral("Memory Range");
                    } else {
                        continue;
                    }

                    res[QStringLiteral("start")] = QString::number(start, 16).toUpper();
                    res[QStringLiteral("end")] = QString::number(end, 16).toUpper();
                    res[QStringLiteral("flags")] = QString::number(flags, 16).toUpper();
                    res[QStringLiteral("displayValue")] =
                        QStringLiteral("%1 - %2")
                            .arg(start, 16, 16, QLatin1Char('0'))
                            .arg(end, 16, 16, QLatin1Char('0'))
                            .toUpper();
                    resources.append(res);
                }
            }
        }
    }
#else
    Q_UNUSED(syspath)
#endif

    return resources;
}

QJsonObject DeviceExport::getDriverInfo(const DeviceInfo &info) {
    QJsonObject driverInfo;

    QString driver = info.driver();
    if (driver.isEmpty()) {
        driverInfo[QStringLiteral("hasDriver")] = false;
        return driverInfo;
    }

    driverInfo[QStringLiteral("hasDriver")] = true;
    driverInfo[QStringLiteral("name")] = driver;

#ifdef Q_OS_LINUX
    // Get module info using modinfo
    QProcess modinfo;
    modinfo.start(QStringLiteral("modinfo"), {driver});
    if (modinfo.waitForFinished(3000)) {
        QString output = QString::fromUtf8(modinfo.readAllStandardOutput());
        QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

        for (const QString &line : lines) {
            auto colonIdx = line.indexOf(QLatin1Char(':'));
            if (colonIdx < 0) {
                continue;
            }

            QString key = line.left(colonIdx).trimmed();
            QString value = line.mid(colonIdx + 1).trimmed();

            if (key == QStringLiteral("filename")) {
                driverInfo[QStringLiteral("filename")] = value;
            } else if (key == QStringLiteral("author")) {
                driverInfo[QStringLiteral("author")] = value;
            } else if (key == QStringLiteral("version")) {
                driverInfo[QStringLiteral("version")] = value;
            } else if (key == QStringLiteral("license")) {
                driverInfo[QStringLiteral("license")] = value;
            } else if (key == QStringLiteral("description")) {
                driverInfo[QStringLiteral("description")] = value;
            } else if (key == QStringLiteral("signer")) {
                driverInfo[QStringLiteral("signer")] = value;
            } else if (key == QStringLiteral("srcversion")) {
                driverInfo[QStringLiteral("srcversion")] = value;
            } else if (key == QStringLiteral("vermagic")) {
                driverInfo[QStringLiteral("vermagic")] = value;
            }
        }

        // Determine if out-of-tree module
        QString filename = driverInfo[QStringLiteral("filename")].toString();
        bool isOutOfTree = !filename.isEmpty() && filename != QStringLiteral("(builtin)") &&
                           !filename.contains(QStringLiteral("/kernel/"));
        driverInfo[QStringLiteral("isOutOfTree")] = isOutOfTree;
        driverInfo[QStringLiteral("isBuiltin")] = (filename == QStringLiteral("(builtin)"));
    }
#elif defined(Q_OS_MACOS)
    // For macOS kext info
    if (driver.contains(QLatin1Char('.'))) {
        driverInfo[QStringLiteral("bundleIdentifier")] = driver;

        QProcess kextstat;
        kextstat.start(QStringLiteral("kextstat"), {QStringLiteral("-b"), driver});
        if (kextstat.waitForFinished(3000)) {
            QString output = QString::fromUtf8(kextstat.readAllStandardOutput());
            if (!output.isEmpty() && output.contains(driver)) {
                QRegularExpression versionRe(QStringLiteral("\\(([^)]+)\\)$"));
                QRegularExpressionMatch match = versionRe.match(output.trimmed());
                if (match.hasMatch()) {
                    driverInfo[QStringLiteral("version")] = match.captured(1);
                }
            }
        }
    }
#elif defined(Q_OS_WIN)
    // Windows driver info from registry
    if (!driver.isEmpty()) {
        QString driverKeyPath =
            QStringLiteral("SYSTEM\\CurrentControlSet\\Control\\Class\\") + driver;
        HKEY hKey;
        std::wstring keyPath = driverKeyPath.toStdWString();

        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) ==
            ERROR_SUCCESS) {
            wchar_t value[256];
            DWORD valueSize = sizeof(value);
            DWORD type;

            if (RegQueryValueExW(hKey,
                                 L"ProviderName",
                                 nullptr,
                                 &type,
                                 reinterpret_cast<LPBYTE>(value),
                                 &valueSize) == ERROR_SUCCESS) {
                if (type == REG_SZ) {
                    driverInfo[QStringLiteral("provider")] = QString::fromWCharArray(value);
                }
            }

            valueSize = sizeof(value);
            if (RegQueryValueExW(hKey,
                                 L"DriverVersion",
                                 nullptr,
                                 &type,
                                 reinterpret_cast<LPBYTE>(value),
                                 &valueSize) == ERROR_SUCCESS) {
                if (type == REG_SZ) {
                    driverInfo[QStringLiteral("version")] = QString::fromWCharArray(value);
                }
            }

            valueSize = sizeof(value);
            if (RegQueryValueExW(hKey,
                                 L"DriverDate",
                                 nullptr,
                                 &type,
                                 reinterpret_cast<LPBYTE>(value),
                                 &valueSize) == ERROR_SUCCESS) {
                if (type == REG_SZ) {
                    driverInfo[QStringLiteral("date")] = QString::fromWCharArray(value);
                }
            }

            RegCloseKey(hKey);
        }
    }
#endif

    return driverInfo;
}
