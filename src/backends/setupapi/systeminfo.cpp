#include "systeminfo.h"
#include "driverinfo.h"
#include "setupapideviceinfo.h"
#include "setupapimanager.h"
#include "setupapimonitor.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

#include <cfgmgr32.h>
#include <windows.h>

bool isComputerEntry(const QString &syspath) {
    // Windows: The computer entry can be empty, the root tree, or the ACPI HAL
    return syspath.isEmpty() || syspath == QStringLiteral("HTREE\\ROOT\\0") ||
           syspath == QStringLiteral("ACPI_HAL\\PNP0C08\\0");
}

QString getComputerDisplayName() {
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    QString baseName;
    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        baseName = QObject::tr("x64-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        baseName = QObject::tr("ARM64-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        baseName = QObject::tr("x86-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        baseName = QObject::tr("ARM-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        baseName = QObject::tr("Itanium-based PC");
        break;
    default:
        baseName = QObject::tr("Standard PC");
        break;
    }

    // All modern Windows PCs use ACPI
    return QObject::tr("ACPI %1").arg(baseName);
}

QString getComputerSyspath() {
    return QStringLiteral("ACPI_HAL\\PNP0C08\\0");
}

void openPrintersSettings() {
    // Windows: Open Devices and Printers control panel
    QProcess::startDetached(QStringLiteral("control"), {QStringLiteral("printers")});
}

BuiltinDriverInfo getBuiltinDriverInfo() {
    BuiltinDriverInfo info;
    info.provider = QStringLiteral("Microsoft Corporation");
    info.signer = QStringLiteral("Microsoft Windows");
    info.builtinMessage = QObject::tr("(Built-in driver)");
    // Windows doesn't expose version for built-in drivers the same way
    return info;
}

DriverFileDetails getDriverFileDetails(const QString &driverPath, const QString &driverName) {
    Q_UNUSED(driverName)
    DriverFileDetails details;

    // Get driver info from the existing function
    DriverInfo info = getDriverInfo(driverPath);

    details.provider = info.author.isEmpty() ? QObject::tr("Unknown") : info.author;
    details.version = info.version.isEmpty() ? QObject::tr("Unknown") : info.version;
    details.copyright = info.license;
    details.signer = info.signer.isEmpty() ? QObject::tr("Not signed") : info.signer;

    return details;
}

QString formatDriverPath(const QString &path) {
    return QDir::toNativeSeparators(path);
}

QString getDeviceDisplayName(const DeviceInfo &info) {
    // On Windows, prefer FriendlyName, then DeviceDesc
    QString friendlyName = info.propertyValue("FriendlyName");
    if (!friendlyName.isEmpty()) {
        return friendlyName;
    }

    QString deviceDesc = info.propertyValue("DeviceDesc");
    if (!deviceDesc.isEmpty()) {
        // DeviceDesc may have format "Description;Provider" - take first part
        int semicolon = deviceDesc.indexOf(QLatin1Char(';'));
        if (semicolon > 0) {
            return deviceDesc.left(semicolon);
        }
        return deviceDesc;
    }

    return info.name();
}

bool hasDriverInfo(const DeviceInfo &info) {
    // On Windows, show driver info if there's a driver key or service
    return !info.driver().isEmpty() || !info.propertyValue("Service").isEmpty();
}

QString getKernelVersion() {
    // Get Windows version using RtlGetVersion
    OSVERSIONINFOEXW osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    typedef NTSTATUS(WINAPI * RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll) {
        auto rtlGetVersion =
            reinterpret_cast<RtlGetVersionFunc>(GetProcAddress(ntdll, "RtlGetVersion"));
        if (rtlGetVersion) {
            rtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi));
            return QStringLiteral("%1.%2.%3")
                .arg(osvi.dwMajorVersion)
                .arg(osvi.dwMinorVersion)
                .arg(osvi.dwBuildNumber);
        }
    }

    return {};
}

QString getKernelBuildDate() {
    // Windows: Return OS install date from registry
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS) {
        DWORD installDate = 0;
        DWORD size = sizeof(installDate);
        if (RegQueryValueExW(hKey,
                             L"InstallDate",
                             nullptr,
                             nullptr,
                             reinterpret_cast<LPBYTE>(&installDate),
                             &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            // InstallDate is a Unix timestamp
            QDateTime dt = QDateTime::fromSecsSinceEpoch(installDate);
            return QLocale().toString(dt.date(), QLocale::ShortFormat);
        }
        RegCloseKey(hKey);
    }
    return {};
}

QString translateDevicePath(const QString &devpath) {
    if (devpath.isEmpty()) {
        return {};
    }

    // Windows: Parse device instance ID paths
    // Format: BUS\VEN_XXXX&DEV_YYYY&SUBSYS_ZZZZ&REV_WW\Instance

    // Check for PCI device
    if (devpath.startsWith(QStringLiteral("PCI\\"), Qt::CaseInsensitive)) {
        QRegularExpression pciVenDevRe(QStringLiteral("VEN_([0-9A-Fa-f]{4})&DEV_([0-9A-Fa-f]{4})"));
        auto match = pciVenDevRe.match(devpath);
        if (match.hasMatch()) {
            return QObject::tr("On PCI bus");
        }
        return QObject::tr("PCI device");
    }

    // Check for USB device
    if (devpath.startsWith(QStringLiteral("USB\\"), Qt::CaseInsensitive)) {
        QRegularExpression usbVidPidRe(QStringLiteral("VID_([0-9A-Fa-f]{4})&PID_([0-9A-Fa-f]{4})"));
        auto match = usbVidPidRe.match(devpath);
        if (match.hasMatch()) {
            return QObject::tr("On USB bus");
        }
        return QObject::tr("USB device");
    }

    // Check for HID device
    if (devpath.startsWith(QStringLiteral("HID\\"), Qt::CaseInsensitive)) {
        return QObject::tr("HID-compliant device");
    }

    // Check for ACPI device
    if (devpath.startsWith(QStringLiteral("ACPI\\"), Qt::CaseInsensitive) ||
        devpath.startsWith(QStringLiteral("ACPI_HAL\\"), Qt::CaseInsensitive)) {
        return QObject::tr("On ACPI-compliant system");
    }

    // Check for root-enumerated device
    if (devpath.startsWith(QStringLiteral("ROOT\\"), Qt::CaseInsensitive)) {
        return QObject::tr("On system board");
    }

    // Check for storage device
    if (devpath.contains(QStringLiteral("SCSI\\"), Qt::CaseInsensitive) ||
        devpath.contains(QStringLiteral("IDE\\"), Qt::CaseInsensitive) ||
        devpath.contains(QStringLiteral("STORAGE\\"), Qt::CaseInsensitive)) {
        return QObject::tr("On storage controller");
    }

    // Check for display device
    if (devpath.startsWith(QStringLiteral("DISPLAY\\"), Qt::CaseInsensitive)) {
        return QObject::tr("On display adapter");
    }

    // Check for software device
    if (devpath.startsWith(QStringLiteral("SWD\\"), Qt::CaseInsensitive) ||
        devpath.startsWith(QStringLiteral("SW\\"), Qt::CaseInsensitive)) {
        return QObject::tr("Software device");
    }

    return {};
}

QString getMountPoint(const QString &devnode) {
    if (devnode.isEmpty()) {
        return {};
    }

    // On Windows, volumes have mount points like "C:\" or volume GUIDs
    // Check if this is a volume device
    QString syspath = devnode;
    if (!syspath.contains(QStringLiteral("STORAGE\\VOLUME"), Qt::CaseInsensitive) &&
        !syspath.contains(QStringLiteral("Volume{"), Qt::CaseInsensitive)) {
        return {};
    }

    wchar_t volumeName[MAX_PATH];
    wchar_t pathNames[MAX_PATH];
    DWORD charCount = MAX_PATH;

    HANDLE findHandle = FindFirstVolumeW(volumeName, MAX_PATH);
    if (findHandle == INVALID_HANDLE_VALUE) {
        return {};
    }

    QString result;
    do {
        size_t len = wcslen(volumeName);
        if (len > 0 && volumeName[len - 1] == L'\\') {
            volumeName[len - 1] = L'\0';
        }

        volumeName[len - 1] = L'\\';
        if (GetVolumePathNamesForVolumeNameW(volumeName, pathNames, charCount, &charCount)) {
            if (pathNames[0] != L'\0') {
                result = QString::fromWCharArray(pathNames);
                if (syspath.contains(QString::fromWCharArray(volumeName).mid(10, 38),
                                     Qt::CaseInsensitive)) {
                    FindVolumeClose(findHandle);
                    return result;
                }
            }
        }
    } while (FindNextVolumeW(findHandle, volumeName, MAX_PATH));

    FindVolumeClose(findHandle);
    return {};
}

QString lookupUsbVendor(const QString &vendorId) {
    // Windows doesn't have a standard usb.ids location
    // Return empty - Windows typically shows vendor name from driver INF
    Q_UNUSED(vendorId)
    return {};
}

DeviceEventQuery buildEventQuery(const DeviceInfo &info) {
    DeviceEventQuery query;
    query.syspath = info.syspath();
    query.devnode = info.devnode();
    query.deviceName = info.name();
    // Windows: extract vendor/product IDs from hardware IDs if present
    QString hwId = info.propertyValue("HardwareID");
    // Hardware IDs like "USB\VID_046D&PID_C52B" contain vendor and product IDs
    static QRegularExpression vidRegex(QStringLiteral("VID_([0-9A-Fa-f]{4})"));
    static QRegularExpression pidRegex(QStringLiteral("PID_([0-9A-Fa-f]{4})"));
    auto vidMatch = vidRegex.match(hwId);
    auto pidMatch = pidRegex.match(hwId);
    if (vidMatch.hasMatch()) {
        query.vendorId = vidMatch.captured(1);
    }
    if (pidMatch.hasMatch()) {
        query.modelId = pidMatch.captured(1);
    }
    return query;
}

QStringList queryDeviceEvents(const DeviceEventQuery &query) {
    QStringList events;
    QStringList searchTerms;

    if (!query.vendorId.isEmpty() && !query.modelId.isEmpty()) {
        searchTerms << QStringLiteral("VID_%1").arg(query.vendorId.toUpper());
    }

    if (!query.deviceName.isEmpty() && query.deviceName.length() >= 8) {
        QString nameSearch = query.deviceName.left(20).trimmed();
        auto lastSpace = nameSearch.lastIndexOf(QLatin1Char(' '));
        if (lastSpace > 8) {
            nameSearch = nameSearch.left(lastSpace);
        }
        searchTerms << nameSearch;
    }

    if (searchTerms.isEmpty()) {
        return events;
    }

    // Query System log using wevtutil
    QProcess wevtutil;
    QStringList args;
    args << QStringLiteral("qe") << QStringLiteral("System") << QStringLiteral("/c:100")
         << QStringLiteral("/rd:true") << QStringLiteral("/f:text");

    wevtutil.start(QStringLiteral("wevtutil"), args);
    if (wevtutil.waitForFinished(10000)) {
        QString output = QString::fromUtf8(wevtutil.readAllStandardOutput());
        QStringList lines = output.split(QStringLiteral("\n"), Qt::SkipEmptyParts);

        for (const QString &line : lines) {
            bool matches = false;
            for (const QString &term : searchTerms) {
                if (line.contains(term, Qt::CaseInsensitive)) {
                    matches = true;
                    break;
                }
            }
            if (matches) {
                events << line;
            }
            if (events.size() >= 50) {
                break;
            }
        }
    }

    return events;
}

ParsedEvent parseEventLine(const QString &line) {
    ParsedEvent result;

    // Windows wevtutil text output format
    QRegularExpression winDateRe(
        QStringLiteral("Date:\\s*(\\d{4}-\\d{2}-\\d{2}\\s+\\d{2}:\\d{2}:\\d{2})"));
    QRegularExpressionMatch match = winDateRe.match(line);

    if (match.hasMatch()) {
        QString dateTimeStr = match.captured(1);
        QDateTime dateTime =
            QDateTime::fromString(dateTimeStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        if (dateTime.isValid()) {
            result.timestamp = QLocale::system().toString(dateTime, QLocale::ShortFormat);
        } else {
            result.timestamp = dateTimeStr;
        }
        result.message = line;
    } else {
        QRegularExpression winMsgRe(QStringLiteral("(?:Message|Description):\\s*(.+)"));
        QRegularExpressionMatch msgMatch = winMsgRe.match(line);
        if (msgMatch.hasMatch()) {
            result.message = msgMatch.captured(1).trimmed();
        } else {
            result.message = line;
        }
    }

    return result;
}

QList<ResourceInfo> getDeviceResources(const QString &syspath, const QString &driver) {
    Q_UNUSED(driver)
    QList<ResourceInfo> resources;

    if (syspath.isEmpty()) {
        return resources;
    }

    // Convert device instance ID to DEVINST handle
    std::wstring instanceId = syspath.toStdWString();
    DEVINST devInst;

    CONFIGRET cr = CM_Locate_DevNodeW(&devInst, const_cast<wchar_t *>(instanceId.c_str()), 0);
    if (cr != CR_SUCCESS) {
        return resources;
    }

    // Get the allocated configuration (current resources)
    LOG_CONF logConf;
    cr = CM_Get_First_Log_Conf(&logConf, devInst, ALLOC_LOG_CONF);
    if (cr != CR_SUCCESS) {
        // Try boot config if alloc config not available
        cr = CM_Get_First_Log_Conf(&logConf, devInst, BOOT_LOG_CONF);
        if (cr != CR_SUCCESS) {
            return resources;
        }
    }

    // Enumerate resource descriptors
    RES_DES resDes;
    RES_DES nextResDes;
    RESOURCEID resType;

    cr = CM_Get_Next_Res_Des(&resDes, logConf, ResType_All, &resType, 0);
    while (cr == CR_SUCCESS) {
        ULONG dataSize = 0;
        cr = CM_Get_Res_Des_Data_Size(&dataSize, resDes, 0);

        if (cr == CR_SUCCESS && dataSize > 0) {
            QByteArray data(static_cast<int>(dataSize), '\0');
            cr = CM_Get_Res_Des_Data(resDes, data.data(), dataSize, 0);

            if (cr == CR_SUCCESS) {
                switch (resType) {
                case ResType_IRQ: {
                    auto *irqData = reinterpret_cast<IRQ_RESOURCE *>(data.data());
                    QString setting =
                        QStringLiteral("0x%1 (%2)")
                            .arg(irqData->IRQ_Header.IRQD_Alloc_Num, 8, 16, QLatin1Char('0'))
                            .arg(irqData->IRQ_Header.IRQD_Alloc_Num)
                            .toUpper();
                    resources.append(
                        {QObject::tr("IRQ"), setting, QStringLiteral("preferences-other")});
                    break;
                }
                case ResType_Mem: {
                    auto *memData = reinterpret_cast<MEM_RESOURCE *>(data.data());
                    DWORDLONG start = memData->MEM_Header.MD_Alloc_Base;
                    DWORDLONG end = memData->MEM_Header.MD_Alloc_End;
                    if (start != 0 || end != 0) {
                        QString setting = QStringLiteral("%1 - %2")
                                              .arg(start, 16, 16, QLatin1Char('0'))
                                              .arg(end, 16, 16, QLatin1Char('0'))
                                              .toUpper();
                        resources.append({QObject::tr("Memory Range"),
                                          setting,
                                          QStringLiteral("drive-harddisk")});
                    }
                    break;
                }
                case ResType_IO: {
                    auto *ioData = reinterpret_cast<IO_RESOURCE *>(data.data());
                    DWORDLONG start = ioData->IO_Header.IOD_Alloc_Base;
                    DWORDLONG end = ioData->IO_Header.IOD_Alloc_End;
                    if (start != 0 || end != 0) {
                        QString setting = QStringLiteral("%1 - %2")
                                              .arg(start, 8, 16, QLatin1Char('0'))
                                              .arg(end, 8, 16, QLatin1Char('0'))
                                              .toUpper();
                        resources.append(
                            {QObject::tr("I/O Range"), setting, QStringLiteral("drive-harddisk")});
                    }
                    break;
                }
                case ResType_DMA: {
                    auto *dmaData = reinterpret_cast<DMA_RESOURCE *>(data.data());
                    QString setting = QString::number(dmaData->DMA_Header.DD_Alloc_Chan);
                    resources.append(
                        {QObject::tr("DMA"), setting, QStringLiteral("preferences-other")});
                    break;
                }
                default:
                    break;
                }
            }
        }

        cr = CM_Get_Next_Res_Des(&nextResDes, resDes, ResType_All, &resType, 0);
        CM_Free_Res_Des_Handle(resDes);
        resDes = nextResDes;
    }

    CM_Free_Log_Conf_Handle(logConf);

    return resources;
}

QList<DmaChannelInfo> getSystemDmaChannels() {
    // TODO: Implement Windows system DMA enumeration
    return {};
}

QList<IoPortInfo> getSystemIoPorts() {
    // TODO: Implement Windows system I/O port enumeration
    return {};
}

QList<IrqInfo> getSystemIrqs() {
    // TODO: Implement Windows system IRQ enumeration
    return {};
}

QList<MemoryRangeInfo> getSystemMemoryRanges() {
    // TODO: Implement Windows system memory range enumeration
    return {};
}

QList<PropertyMapping> getDevicePropertyMappings() {
    return {
        {QObject::tr("Device description"), PropertyKeys::deviceDescription(), false},
        {QObject::tr("Hardware IDs"), QStringLiteral("HardwareID"), true},
        {QObject::tr("Compatible IDs"), QStringLiteral("CompatibleIDs"), true},
        {QObject::tr("Device class"), QStringLiteral("Class"), false},
        {QObject::tr("Device class GUID"), QStringLiteral("ClassGuid"), false},
        {QObject::tr("Driver key"), QStringLiteral("Driver"), false},
        {QObject::tr("Device instance path"), QStringLiteral("DeviceInstanceId"), false},
        {QObject::tr("Location paths"), QStringLiteral("LocationPaths"), true},
        {QObject::tr("Location info"), QStringLiteral("LocationInformation"), false},
        {QObject::tr("Physical device object name"), QStringLiteral("PDOName"), false},
        {QObject::tr("Bus relations"), QStringLiteral("BusRelations"), true},
        {QObject::tr("Parent"), PropertyKeys::parentSyspath(), false},
        {QObject::tr("Children"), PropertyKeys::children(), true},
        {QObject::tr("Manufacturer"), QStringLiteral("Manufacturer"), false},
        {QObject::tr("Model"), QStringLiteral("FriendlyName"), false},
        {QObject::tr("Service"), QStringLiteral("Service"), false},
        {QObject::tr("INF name"), QStringLiteral("InfPath"), false},
        {QObject::tr("INF section"), QStringLiteral("InfSection"), false},
        {QObject::tr("Syspath"), PropertyKeys::syspath(), false},
        {QObject::tr("Mount point"), PropertyKeys::mountPoint(), false},
    };
}

QStringList convertToHardwareIds(const QString &propertyKey, const QString &value) {
    // On Windows, hardware IDs are already in Windows format
    // Just return the value if it's a hardware ID property
    if ((propertyKey == QStringLiteral("HardwareID") ||
         propertyKey == QStringLiteral("CompatibleIDs")) &&
        !value.isEmpty()) {
        // Hardware IDs may be multi-string, already separated
        return value.split(QLatin1Char('\0'), Qt::SkipEmptyParts);
    }
    return {};
}

BasicDriverInfo getBasicDriverInfo(const QString &driver) {
    BasicDriverInfo info;
    info.provider = QStringLiteral("Microsoft");
    info.version = getKernelVersion();
    info.signer = QStringLiteral("Microsoft");
    info.date = getKernelBuildDate();
    info.hasDriverFiles = false;

    if (driver.isEmpty()) {
        return info;
    }

    QString driverKeyPath = QStringLiteral("SYSTEM\\CurrentControlSet\\Control\\Class\\") + driver;
    HKEY hKey;
    std::wstring keyPath = driverKeyPath.toStdWString();

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return info;
    }

    wchar_t value[256];
    DWORD valueSize = sizeof(value);
    DWORD type;

    if (RegQueryValueExW(
            hKey, L"ProviderName", nullptr, &type, reinterpret_cast<LPBYTE>(value), &valueSize) ==
            ERROR_SUCCESS &&
        type == REG_SZ) {
        info.provider = QString::fromWCharArray(value);
        info.hasDriverFiles = true;
    }

    valueSize = sizeof(value);
    if (RegQueryValueExW(
            hKey, L"DriverVersion", nullptr, &type, reinterpret_cast<LPBYTE>(value), &valueSize) ==
            ERROR_SUCCESS &&
        type == REG_SZ) {
        info.version = QString::fromWCharArray(value);
    }

    valueSize = sizeof(value);
    if (RegQueryValueExW(
            hKey, L"DriverDate", nullptr, &type, reinterpret_cast<LPBYTE>(value), &valueSize) ==
            ERROR_SUCCESS &&
        type == REG_SZ) {
        info.date = QString::fromWCharArray(value);
    }

    valueSize = sizeof(value);
    if (RegQueryValueExW(
            hKey, L"InfPath", nullptr, &type, reinterpret_cast<LPBYTE>(value), &valueSize) ==
            ERROR_SUCCESS &&
        type == REG_SZ) {
        QString infPath = QString::fromWCharArray(value);
        if (infPath.startsWith(QStringLiteral("oem"), Qt::CaseInsensitive)) {
            info.signer = info.provider;
        } else {
            info.signer = QStringLiteral("Microsoft Windows");
        }
    }

    RegCloseKey(hKey);
    return info;
}

QString getCategoryDisplayName(DeviceCategory category, const QString &fallback) {
    switch (category) {
    case DeviceCategory::AudioInputsAndOutputs:
        return QObject::tr("Audio inputs and outputs");
    case DeviceCategory::Batteries:
        return QObject::tr("Batteries");
    case DeviceCategory::Computer:
        return QObject::tr("Computer");
    case DeviceCategory::DiskDrives:
        return QObject::tr("Disk drives");
    case DeviceCategory::DisplayAdapters:
        return QObject::tr("Display adapters");
    case DeviceCategory::DvdCdromDrives:
        return QObject::tr("DVD/CD-ROM drives");
    case DeviceCategory::HumanInterfaceDevices:
        return QObject::tr("Human Interface Devices");
    case DeviceCategory::Keyboards:
        return QObject::tr("Keyboards");
    case DeviceCategory::MiceAndOtherPointingDevices:
        return QObject::tr("Mice and other pointing devices");
    case DeviceCategory::NetworkAdapters:
        return QObject::tr("Network adapters");
    case DeviceCategory::SoftwareDevices:
        return QObject::tr("Software devices");
    case DeviceCategory::SoundVideoAndGameControllers:
        return QObject::tr("Sound, video and game controllers");
    case DeviceCategory::StorageControllers:
        return QObject::tr("Storage controllers");
    case DeviceCategory::StorageVolumes:
        return QObject::tr("Storage volumes");
    case DeviceCategory::SystemDevices:
        return QObject::tr("System devices");
    case DeviceCategory::UniversalSerialBusControllers:
        return QObject::tr("Universal Serial Bus controllers");
    case DeviceCategory::Unknown:
    default:
        return fallback.isEmpty() ? QObject::tr("Unknown") : fallback;
    }
}

QString getDeviceManufacturer(const DeviceInfo &info) {
    // Storage volumes don't have a manufacturer
    if (info.category() == DeviceCategory::StorageVolumes) {
        return {};
    }

    // On Windows, manufacturer is typically available directly
    QString manufacturer = info.propertyValue("Manufacturer");
    return manufacturer;
}

UnameInfo getUnameInfo() {
    // Windows doesn't have uname
    return {};
}

QHash<QString, QString> getDistributionInfo() {
    // Windows doesn't have /etc/os-release
    return {};
}

QHash<QString, QString> getExportDeviceProperties(const DeviceInfo &info) {
    QHash<QString, QString> properties;

    // Windows-specific properties
    auto addIfNotEmpty = [&](const QString &key, const char *propName) {
        QString value = info.propertyValue(propName);
        if (!value.isEmpty()) {
            properties[key] = value;
        }
    };

    addIfNotEmpty(QStringLiteral("HardwareID"), "HardwareID");
    addIfNotEmpty(QStringLiteral("CompatibleIDs"), "CompatibleIDs");
    addIfNotEmpty(QStringLiteral("Class"), "Class");
    addIfNotEmpty(QStringLiteral("ClassGuid"), "ClassGuid");
    addIfNotEmpty(QStringLiteral("Manufacturer"), "Manufacturer");
    addIfNotEmpty(QStringLiteral("FriendlyName"), "FriendlyName");
    addIfNotEmpty(QStringLiteral("DeviceDesc"), "DeviceDesc");
    addIfNotEmpty(QStringLiteral("Driver"), "Driver");
    addIfNotEmpty(QStringLiteral("Service"), "Service");

    return properties;
}

QList<ExportResourceInfo> getExportDeviceResources(const QString &syspath) {
    Q_UNUSED(syspath)
    // TODO: Implement Windows resource enumeration for export
    return {};
}

ExportDriverInfo getExportDriverInfo(const DeviceInfo &info) {
    ExportDriverInfo driverInfo;

    QString driver = info.driver();
    if (driver.isEmpty()) {
        driverInfo.hasDriver = false;
        return driverInfo;
    }

    driverInfo.hasDriver = true;
    driverInfo.name = driver;

    // Windows driver info from registry
    QString driverKeyPath = QStringLiteral("SYSTEM\\CurrentControlSet\\Control\\Class\\") + driver;
    HKEY hKey;
    std::wstring keyPath = driverKeyPath.toStdWString();

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
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
                driverInfo.provider = QString::fromWCharArray(value);
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
                driverInfo.version = QString::fromWCharArray(value);
            }
        }

        valueSize = sizeof(value);
        if (RegQueryValueExW(
                hKey, L"DriverDate", nullptr, &type, reinterpret_cast<LPBYTE>(value), &valueSize) ==
            ERROR_SUCCESS) {
            if (type == REG_SZ) {
                driverInfo.date = QString::fromWCharArray(value);
            }
        }

        RegCloseKey(hKey);
    }

    return driverInfo;
}

QHash<QString, QString> getSystemResourcesRaw() {
    // Windows doesn't have /proc filesystem
    return {};
}

// Global SetupAPI manager for enumeration
static SetupApiManager &getGlobalManager() {
    static SetupApiManager manager;
    return manager;
}

QList<DeviceInfo> enumerateAllDevices() {
    QList<DeviceInfo> devices;
    auto &manager = getGlobalManager();

    manager.enumerateAllDevices([&devices](HDEVINFO devInfo, SP_DEVINFO_DATA *devInfoData) {
        auto *d = createDeviceInfo(devInfo, devInfoData);
        if (d) {
            devices.emplaceBack(d);
        }
    });

    return devices;
}

QObject *createDeviceMonitor(QObject *parent) {
    return new SetupApiMonitor(parent);
}
