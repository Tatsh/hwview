#include <sys/mount.h>
#include <sys/utsname.h>
#include <unistd.h>

#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QProcess>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

#include "driverinfo.h"
#include "iokitdeviceinfo.h"
#include "iokitmanager.h"
#include "iokitmonitor.h"
#include "systeminfo.h"

bool isComputerEntry(const QString &syspath) {
    return syspath == QStringLiteral("IOService:/");
}

QString getComputerDisplayName() {
    // Try to get specific Mac model name using sysctl
    QProcess sysctl;
    sysctl.start(QStringLiteral("sysctl"), {QStringLiteral("-n"), QStringLiteral("hw.model")});
    if (sysctl.waitForFinished(1000)) {
        auto model = QString::fromUtf8(sysctl.readAllStandardOutput()).trimmed();
        if (!model.isEmpty()) {
            if (model.startsWith(QStringLiteral("Mac"))) {
                if (model.contains(QStringLiteral("BookPro"))) {
                    return QObject::tr("MacBook Pro");
                }
                if (model.contains(QStringLiteral("BookAir"))) {
                    return QObject::tr("MacBook Air");
                }
                if (model.contains(QStringLiteral("Book"))) {
                    return QObject::tr("MacBook");
                }
                if (model.contains(QStringLiteral("Pro"))) {
                    return QObject::tr("Mac Pro");
                }
                if (model.contains(QStringLiteral("mini"))) {
                    return QObject::tr("Mac mini");
                }
                if (model.contains(QStringLiteral("Studio"))) {
                    return QObject::tr("Mac Studio");
                }
                return QObject::tr("Mac (%1)").arg(model);
            }
            if (model.startsWith(QStringLiteral("iMac"))) {
                return QObject::tr("iMac");
            }
            return model;
        }
    }

    // Fallback based on processor
#if defined(Q_PROCESSOR_ARM_64)
    return QObject::tr("Apple Silicon Mac");
#elif defined(Q_PROCESSOR_X86_64)
    return QObject::tr("Intel-based Mac");
#else
    return QObject::tr("Mac");
#endif // Q_PROCESSOR_ARM_64
}

QString getComputerSyspath() {
    return QStringLiteral("IOService:/");
}

QString getHostname() {
    char buffer[256];
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        return QString::fromLocal8Bit(buffer);
    }
    return QStringLiteral("unknown");
}

void openPrintersSettings() {
    // macOS: Open System Preferences/Settings to Printers
    QDesktopServices::openUrl(
        QUrl(QStringLiteral("x-apple.systempreferences:com.apple.preference.printfax")));
}

BuiltinDriverInfo getBuiltinDriverInfo() {
    BuiltinDriverInfo info;
    info.provider = QStringLiteral("Apple Inc.");
    info.copyright = QStringLiteral("Apple Public Source License");
    info.signer = QStringLiteral("Apple Inc.");
    info.builtinMessage = QObject::tr("(Built-in kernel driver)");

    struct utsname buffer;
    if (uname(&buffer) == 0) {
        info.version = QString::fromLocal8Bit(buffer.release);
    }

    return info;
}

DriverFileDetails getDriverFileDetails(const QString &driverPath, const QString &driverName) {
    Q_UNUSED(driverName)
    DriverFileDetails details;

    // Get driver info from the existing function
    DriverInfo info = getDriverInfo(driverPath);

    auto isSystemDriver = driverPath.startsWith(QStringLiteral("/System/"));

    if (!info.author.isEmpty()) {
        details.provider = info.author;
    } else if (isSystemDriver) {
        details.provider = QStringLiteral("Apple Inc.");
    }

    if (!info.version.isEmpty()) {
        details.version = info.version;
    } else {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            details.version = QString::fromLocal8Bit(buffer.release);
        }
    }

    if (!info.license.isEmpty()) {
        details.copyright = info.license;
    } else if (isSystemDriver) {
        details.copyright = QStringLiteral("Apple Public Source License");
    }

    if (!info.signer.isEmpty()) {
        details.signer = info.signer;
    } else if (isSystemDriver) {
        details.signer = QStringLiteral("Apple Inc.");
    }

    return details;
}

QString formatDriverPath(const QString &path) {
    return path;
}

QString getDeviceDisplayName(const DeviceInfo &info) {
    auto name = info.name();

    // For storage volumes, try volume name
    if (info.category() == DeviceCategory::StorageVolumes) {
        auto volumeName = info.propertyValue("VolumeName");
        if (!volumeName.isEmpty()) {
            return volumeName;
        }
    }

    // For batteries, use friendly name
    if (info.category() == DeviceCategory::Batteries) {
        auto ioClass = info.propertyValue("IOClass");
        if (ioClass.contains(QStringLiteral("Battery"))) {
            return QObject::tr("Built-in Battery");
        }
        if (ioClass.contains(QStringLiteral("AC"))) {
            return QObject::tr("AC Power Adapter");
        }
    }

    return name;
}

bool hasDriverInfo(const DeviceInfo &info) {
    // On macOS, all IOKit devices effectively have drivers (IOKit classes)
    // Show driver info if there's a driver/IOClass
    return !info.driver().isEmpty() || !info.propertyValue("IOClass").isEmpty();
}

QString getKernelVersion() {
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        return QString::fromLocal8Bit(buffer.release);
    }
    return {};
}

QString getKernelBuildDate() {
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        auto version = QString::fromLocal8Bit(buffer.version);
        // Format: "Darwin Kernel Version X.X.X: Day Mon DD HH:MM:SS TZ YYYY; root:xnu-..."
        QRegularExpression dateRe(
            QStringLiteral(R"((Mon|Tue|Wed|Thu|Fri|Sat|Sun)\s+)"
                           R"((Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec))"
                           R"(\s+)"
                           R"((\d{1,2})\s+)"
                           R"((\d{2}):(\d{2}):(\d{2})\s+)"
                           R"(\w+\s+)" // Timezone
                           R"((\d{4}))"));

        auto match = dateRe.match(version);
        if (match.hasMatch()) {
            auto monthStr = match.captured(2);
            auto day = match.captured(3).toInt();
            auto year = match.captured(7).toInt();

            static const QStringList months = {QStringLiteral("Jan"),
                                               QStringLiteral("Feb"),
                                               QStringLiteral("Mar"),
                                               QStringLiteral("Apr"),
                                               QStringLiteral("May"),
                                               QStringLiteral("Jun"),
                                               QStringLiteral("Jul"),
                                               QStringLiteral("Aug"),
                                               QStringLiteral("Sep"),
                                               QStringLiteral("Oct"),
                                               QStringLiteral("Nov"),
                                               QStringLiteral("Dec")};
            auto month = static_cast<int>(months.indexOf(monthStr)) + 1;

            if (month > 0) {
                QDate date(year, month, day);
                if (date.isValid()) {
                    return QLocale().toString(date, QLocale::ShortFormat);
                }
            }
        }
        return version;
    }
    return {};
}

QString translateDevicePath(const QString &devpath) {
    if (devpath.isEmpty()) {
        return {};
    }

    // macOS: Parse IORegistry paths
    // Format: IOService:/AppleARMPE/arm-io@10F00000/AppleT810xIO/usb-drd1@2280000/...

    // Check for USB device
    if (devpath.contains(QStringLiteral("USB")) || devpath.contains(QStringLiteral("usb"))) {
        static const QRegularExpression usbLocationRe(
            QStringLiteral("usb[^/]*/([^@/]+)@([0-9a-fA-F]+)"));
        auto match = usbLocationRe.match(devpath);
        if (match.hasMatch()) {
            return QObject::tr("On USB bus");
        }
        return QObject::tr("On USB bus");
    }

    // Check for PCI device
    if (devpath.contains(QStringLiteral("PCI")) || devpath.contains(QStringLiteral("pci"))) {
        return QObject::tr("On PCI bus");
    }

    // Check for Thunderbolt
    if (devpath.contains(QStringLiteral("Thunderbolt"))) {
        return QObject::tr("On Thunderbolt bus");
    }

    // Check for built-in devices
    if (devpath.contains(QStringLiteral("AppleARMPE")) ||
        devpath.contains(QStringLiteral("arm-io"))) {
        return QObject::tr("On system board");
    }

    // Check for Apple internal
    if (devpath.contains(QStringLiteral("Apple"))) {
        return QObject::tr("Built-in");
    }

    return {};
}

QString getMountPoint(const QString &devnode) {
    if (devnode.isEmpty()) {
        return {};
    }

    QFileInfo devnodeInfo(devnode);
    auto canonicalDevnode = devnodeInfo.canonicalFilePath();
    if (canonicalDevnode.isEmpty()) {
        canonicalDevnode = devnode;
    }

    struct statfs *mounts;
    auto numMounts = getmntinfo(&mounts, MNT_NOWAIT);

    for (auto i = 0; i < numMounts; ++i) {
        auto mountDevice = QString::fromUtf8(mounts[i].f_mntfromname);
        auto mountPoint = QString::fromUtf8(mounts[i].f_mntonname);

        QFileInfo mountDevInfo(mountDevice);
        auto canonicalMountDev = mountDevInfo.canonicalFilePath();
        if (canonicalMountDev.isEmpty()) {
            canonicalMountDev = mountDevice;
        }

        if (mountDevice == devnode || mountDevice == canonicalDevnode ||
            canonicalMountDev == devnode || canonicalMountDev == canonicalDevnode) {
            return mountPoint;
        }
    }

    return {};
}

QString lookupUsbVendor(const QString &vendorId) {
    static QHash<QString, QString> vendorCache;
    static auto initialized = false;

    if (!initialized) {
        initialized = true;

        static const QStringList usbIdsLocations = {
            // Homebrew locations (Apple Silicon and Intel)
            QStringLiteral("/opt/homebrew/share/hwdata/usb.ids"),
            QStringLiteral("/usr/local/share/hwdata/usb.ids"),
            // MacPorts
            QStringLiteral("/opt/local/share/hwdata/usb.ids"),
        };

        for (const auto &path : usbIdsLocations) {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }

            QTextStream in(&file);
            while (!in.atEnd()) {
                auto line = in.readLine();
                if (line.length() >= 6 && !line.startsWith(QLatin1Char('\t')) &&
                    !line.startsWith(QLatin1Char('#'))) {
                    auto lineVendorId = line.left(4).toLower();
                    auto name = line.mid(6).trimmed();
                    if (!name.isEmpty()) {
                        vendorCache.insert(lineVendorId, name);
                    }
                }
            }
            file.close();

            if (!vendorCache.isEmpty()) {
                break;
            }
        }
    }

    return vendorCache.value(vendorId.toLower());
}

DeviceEventQuery buildEventQuery(const DeviceInfo &info) {
    DeviceEventQuery query;
    query.syspath = info.syspath();
    query.devnode = info.devnode();
    query.deviceName = info.name();
    // macOS uses different property names for USB IDs
    query.vendorId = info.propertyValue("idVendor");
    query.modelId = info.propertyValue("idProduct");
    return query;
}

QStringList queryDeviceEvents(const DeviceEventQuery &query) {
    QStringList events;
    QStringList searchTerms;

    if (!query.vendorId.isEmpty() && !query.modelId.isEmpty()) {
        searchTerms << QStringLiteral("idVendor=%1").arg(query.vendorId.toLower());
    }

    if (!query.deviceName.isEmpty() && query.deviceName.length() >= 8) {
        auto nameSearch = query.deviceName.left(20).trimmed();
        auto lastSpace = nameSearch.lastIndexOf(QLatin1Char(' '));
        if (lastSpace > 8) {
            nameSearch = nameSearch.left(lastSpace);
        }
        searchTerms << nameSearch;
    }

    if (!query.devnode.isEmpty()) {
        auto shortName = query.devnode;
        if (shortName.startsWith(QStringLiteral("/dev/"))) {
            shortName = shortName.mid(5);
        }
        if (shortName.length() >= 3) {
            searchTerms << shortName;
        }
    }

    if (searchTerms.isEmpty()) {
        return events;
    }

    QProcess logShow;
    QStringList args;
    args << QStringLiteral("show") << QStringLiteral("--predicate")
         << QStringLiteral("subsystem == 'com.apple.iokit' OR "
                           "subsystem == 'com.apple.kernel' OR "
                           "category == 'IOKit'")
         << QStringLiteral("--last") << QStringLiteral("1h") << QStringLiteral("--style")
         << QStringLiteral("compact");

    logShow.start(QStringLiteral("log"), args);
    if (logShow.waitForFinished(10000)) {
        auto output = QString::fromUtf8(logShow.readAllStandardOutput());
        auto lines = output.split(QStringLiteral("\n"), Qt::SkipEmptyParts);

        for (const auto &line : lines) {
            auto matches = false;
            for (const auto &term : searchTerms) {
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

    // macOS 'log show --style compact' format:
    // "YYYY-MM-DD HH:MM:SS.mmm Df subsystem[pid]: message"
    QRegularExpression macLogRe(
        QStringLiteral(R"(^(\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}\.\d+)\s+\w+\s+)"));
    auto match = macLogRe.match(line);

    if (match.hasMatch()) {
        auto dateTimeStr = match.captured(1);
        auto remainder = line.mid(match.capturedEnd());

        auto dateTime =
            QDateTime::fromString(dateTimeStr, QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
        if (dateTime.isValid()) {
            result.timestamp = QLocale::system().toString(dateTime, QLocale::ShortFormat);
        } else {
            result.timestamp = dateTimeStr;
        }

        auto colonIdx = remainder.indexOf(QStringLiteral(": "));
        if (colonIdx != -1) {
            result.message = remainder.mid(colonIdx + 2).trimmed();
        } else {
            result.message = remainder.trimmed();
        }
    } else {
        result.message = line;
    }

    return result;
}

QList<ResourceInfo> getDeviceResources(const QString &syspath, const QString &driver) {
    Q_UNUSED(driver)
    QList<ResourceInfo> resources;

    if (syspath.isEmpty()) {
        return resources;
    }

    // Use ioreg to get device properties including resources
    // Format: ioreg -r -d 1 -a -c <classname> or by path
    QProcess ioreg;
    QStringList args;

    // Try to extract the IOKit class or entry name from the path
    // IORegistry paths look like: IOService:/AppleARMPE/arm-io@.../device@...
    QString entryName;
    if (syspath.contains(QLatin1Char('/'))) {
        entryName = syspath.section(QLatin1Char('/'), -1);
        // Remove @address suffix if present
        auto atIdx = entryName.indexOf(QLatin1Char('@'));
        if (atIdx > 0) {
            entryName = entryName.left(atIdx);
        }
    }

    if (entryName.isEmpty()) {
        return resources;
    }

    // Query ioreg for this specific entry
    args << QStringLiteral("-r") << QStringLiteral("-d") << QStringLiteral("1")
         << QStringLiteral("-n") << entryName;

    ioreg.start(QStringLiteral("ioreg"), args);
    if (!ioreg.waitForFinished(3000)) {
        return resources;
    }

    auto output = QString::fromUtf8(ioreg.readAllStandardOutput());

    // Parse ioreg output for resource information
    // Look for patterns like:
    // "IOInterruptSpecifiers" = (...)
    // "IODeviceMemory" = (...)
    // "assigned-addresses" = <...>

    // Extract interrupts
    QRegularExpression interruptRe(
        QStringLiteral("\"(?:IOInterruptSpecifiers|interrupts)\"\\s*=\\s*\\(([^)]+)\\)"));
    auto interruptMatch = interruptRe.match(output);
    if (interruptMatch.hasMatch()) {
        auto interruptData = interruptMatch.captured(1);
        // Count interrupt entries (each <...> block is one interrupt)
        auto irqCount = interruptData.count(QLatin1Char('<'));
        if (irqCount > 0) {
            for (auto i = 0; i < irqCount; ++i) {
                resources.append({QObject::tr("Interrupt"),
                                  QObject::tr("IRQ %1").arg(i),
                                  QStringLiteral("preferences-other")});
            }
        }
    }

    // Extract memory ranges from IODeviceMemory or reg property
    QRegularExpression memoryRe(QStringLiteral(R"("(?:IODeviceMemory|reg)"\s*=\s*\(([^)]+)\))"));
    auto memoryMatch = memoryRe.match(output);
    if (memoryMatch.hasMatch()) {
        auto memData = memoryMatch.captured(1);
        // Parse hex values - format varies but typically contains address/size pairs
        QRegularExpression hexRe(QStringLiteral(R"(<([0-9a-fA-F\s]+)>)"));
        auto hexIt = hexRe.globalMatch(memData);
        auto memIndex = 0;
        while (hexIt.hasNext()) {
            auto hexMatch = hexIt.next();
            auto hexStr = hexMatch.captured(1).simplified().replace(QLatin1Char(' '), QString());
            if (hexStr.length() >= 8) {
                // Format as memory range
                bool ok;
                auto addr = hexStr.left(16).toULongLong(&ok, 16);
                if (ok && addr != 0) {
                    auto setting =
                        QStringLiteral("0x%1").arg(addr, 16, 16, QLatin1Char('0')).toUpper();
                    resources.append(
                        {QObject::tr("Memory Range"), setting, QStringLiteral("drive-harddisk")});
                    ++memIndex;
                    if (memIndex >= 8) {
                        break; // Limit to avoid too many entries
                    }
                }
            }
        }
    }

    // Check for DMA channels (rare on modern macOS)
    if (output.contains(QStringLiteral("dma-channels")) ||
        output.contains(QStringLiteral("IODMAChannels"))) {
        resources.append(
            {QObject::tr("DMA"), QObject::tr("Available"), QStringLiteral("preferences-other")});
    }

    return resources;
}

QList<DmaChannelInfo> getSystemDmaChannels() {
    // DMA is not commonly exposed on macOS
    return {};
}

QList<IoPortInfo> getSystemIoPorts() {
    // I/O ports are not exposed in the same way on macOS
    return {};
}

QList<IrqInfo> getSystemIrqs() {
    // IRQ information is not directly accessible on macOS
    return {};
}

QList<MemoryRangeInfo> getSystemMemoryRanges() {
    // Memory ranges are not exposed in the same way on macOS
    return {};
}

QList<PropertyMapping> getDevicePropertyMappings() {
    return {
        {QObject::tr("Device description"), PropertyKeys::deviceDescription(), false},
        {QObject::tr("Hardware IDs"), QStringLiteral("IOPropertyMatch"), false},
        {QObject::tr("IOKit class"), QStringLiteral("IOClass"), false},
        {QObject::tr("IOKit class match"), QStringLiteral("IOProviderClass"), false},
        {QObject::tr("Bundle identifier"), QStringLiteral("CFBundleIdentifier"), false},
        {QObject::tr("Device instance path"), QStringLiteral("IORegistryEntryPath"), false},
        {QObject::tr("Parent"), PropertyKeys::parentSyspath(), false},
        {QObject::tr("Vendor"), QStringLiteral("kUSBVendorString"), false},
        {QObject::tr("Vendor ID"), QStringLiteral("idVendor"), false},
        {QObject::tr("Product"), QStringLiteral("kUSBProductString"), false},
        {QObject::tr("Product ID"), QStringLiteral("idProduct"), false},
        {QObject::tr("Serial number"), QStringLiteral("kUSBSerialNumberString"), false},
        {QObject::tr("Device speed"), QStringLiteral("Device Speed"), false},
        {QObject::tr("Port info"), QStringLiteral("PortInfo"), false},
        {QObject::tr("Location ID"), QStringLiteral("locationID"), false},
        {QObject::tr("Syspath"), PropertyKeys::syspath(), false},
        {QObject::tr("Mount point"), PropertyKeys::mountPoint(), false},
    };
}

QStringList convertToHardwareIds(const QString &propertyKey, const QString &value) {
    QStringList result;

    // On macOS, we can convert USB vendor/product IDs to Windows format
    if (propertyKey == QStringLiteral("idVendor") || propertyKey == QStringLiteral("idProduct")) {
        // These are typically decimal on macOS, would need both values to form hardware ID
        // For now, return empty - the full conversion would need both values together
        return result;
    }

    // For IOPropertyMatch containing USB IDs
    if (propertyKey == QStringLiteral("IOPropertyMatch") && !value.isEmpty()) {
        // Try to extract vendor/product from property match data
        QRegularExpression vendorRe(QStringLiteral(R"(idVendor\s*=\s*(\d+))"));
        QRegularExpression productRe(QStringLiteral(R"(idProduct\s*=\s*(\d+))"));

        auto vendorMatch = vendorRe.match(value);
        auto productMatch = productRe.match(value);

        if (vendorMatch.hasMatch() && productMatch.hasMatch()) {
            bool ok1, ok2;
            auto vendorId = vendorMatch.captured(1).toInt(&ok1);
            auto productId = productMatch.captured(1).toInt(&ok2);
            if (ok1 && ok2) {
                result << QStringLiteral(R"(USB\VID_%1&PID_%2)")
                              .arg(vendorId, 4, 16, QLatin1Char('0'))
                              .arg(productId, 4, 16, QLatin1Char('0'))
                              .toUpper();
            }
        }
    }

    return result;
}

BasicDriverInfo getBasicDriverInfo(const QString &driver) {
    BasicDriverInfo info;
    info.provider = QStringLiteral("Apple Inc.");
    info.version = getKernelVersion();
    info.signer = QStringLiteral("Apple Inc.");
    info.date = getKernelBuildDate();
    info.hasDriverFiles = false;

    if (driver.isEmpty() || !driver.contains(QLatin1Char('.'))) {
        return info;
    }

    QProcess kextstat;
    kextstat.start(QStringLiteral("kextstat"), {QStringLiteral("-b"), driver});
    if (!kextstat.waitForFinished(3000)) {
        return info;
    }

    auto output = QString::fromUtf8(kextstat.readAllStandardOutput());
    if (output.isEmpty() || !output.contains(driver)) {
        return info;
    }

    info.hasDriverFiles = true;

    QRegularExpression versionRe(QStringLiteral(R"(\(([^)]+)\)$)"));
    auto match = versionRe.match(output.trimmed());
    if (match.hasMatch()) {
        info.version = match.captured(1);
    }

    if (driver.startsWith(QStringLiteral("com.apple."))) {
        info.provider = QStringLiteral("Apple Inc.");
        info.signer = QStringLiteral("Apple Inc.");
    } else {
        auto parts = driver.split(QLatin1Char('.'));
        if (parts.size() >= 2) {
            info.provider = parts.at(1);
            if (!info.provider.isEmpty()) {
                info.provider[0] = info.provider[0].toUpper();
            }
        }
        info.signer = info.provider;
    }

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

    // Try USB vendor string
    auto manufacturer = info.propertyValue("kUSBVendorString");
    if (!manufacturer.isEmpty()) {
        return manufacturer;
    }

    // Try vendor ID lookup
    auto vendorId = info.propertyValue("idVendor");
    if (!vendorId.isEmpty()) {
        bool ok;
        auto vid = vendorId.toInt(&ok);
        if (ok && vid != 0) {
            manufacturer =
                lookupUsbVendor(QString::number(vid, 16).rightJustified(4, QLatin1Char('0')));
            if (!manufacturer.isEmpty()) {
                return manufacturer;
            }
        }
    }

    // For Apple devices
    auto ioClass = info.propertyValue("IOClass");
    if (ioClass.startsWith(QStringLiteral("Apple"))) {
        return QStringLiteral("Apple Inc.");
    }

    return {};
}

UnameInfo getUnameInfo() {
    UnameInfo info;
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        info.sysname = QString::fromLocal8Bit(buffer.sysname);
        info.release = QString::fromLocal8Bit(buffer.release);
        info.version = QString::fromLocal8Bit(buffer.version);
        info.machine = QString::fromLocal8Bit(buffer.machine);
        info.valid = true;
    }
    return info;
}

QHash<QString, QString> getDistributionInfo() {
    // macOS doesn't have /etc/os-release
    return {};
}

QHash<QString, QString> getExportDeviceProperties(const DeviceInfo &info) {
    QHash<QString, QString> properties;

    // macOS-specific properties
    auto addIfNotEmpty = [&](const QString &key, const char *propName) {
        auto value = info.propertyValue(propName);
        if (!value.isEmpty()) {
            properties[key] = value;
        }
    };

    addIfNotEmpty(QStringLiteral("kUSBVendorString"), "kUSBVendorString");
    addIfNotEmpty(QStringLiteral("kUSBProductString"), "kUSBProductString");
    addIfNotEmpty(QStringLiteral("idVendor"), "idVendor");
    addIfNotEmpty(QStringLiteral("idProduct"), "idProduct");
    addIfNotEmpty(QStringLiteral("IOClass"), "IOClass");
    addIfNotEmpty(QStringLiteral("IOProviderClass"), "IOProviderClass");
    addIfNotEmpty(QStringLiteral("CFBundleIdentifier"), "CFBundleIdentifier");

    return properties;
}

QList<ExportResourceInfo> getExportDeviceResources(const QString &syspath) {
    Q_UNUSED(syspath)
    // macOS doesn't expose resources the same way
    return {};
}

ExportDriverInfo getExportDriverInfo(const DeviceInfo &info) {
    ExportDriverInfo driverInfo;

    auto driver = info.driver();
    if (driver.isEmpty()) {
        driverInfo.hasDriver = false;
        return driverInfo;
    }

    driverInfo.hasDriver = true;
    driverInfo.name = driver;

    // For macOS kext info
    if (driver.contains(QLatin1Char('.'))) {
        driverInfo.bundleIdentifier = driver;

        QProcess kextstat;
        kextstat.start(QStringLiteral("kextstat"), {QStringLiteral("-b"), driver});
        if (kextstat.waitForFinished(3000)) {
            auto output = QString::fromUtf8(kextstat.readAllStandardOutput());
            if (!output.isEmpty() && output.contains(driver)) {
                QRegularExpression versionRe(QStringLiteral(R"(\(([^)]+)\)$)"));
                auto match = versionRe.match(output.trimmed());
                if (match.hasMatch()) {
                    driverInfo.version = match.captured(1);
                }
            }
        }
    }

    return driverInfo;
}

QHash<QString, QString> getSystemResourcesRaw() {
    // macOS doesn't have /proc filesystem
    return {};
}

// Global IOKit manager for enumeration
static IOKitManager &getGlobalManager() {
    static IOKitManager manager;
    return manager;
}

QList<DeviceInfo> enumerateAllDevices() {
    QList<DeviceInfo> devices;
    auto &manager = getGlobalManager();

    manager.enumerateAllDevices([&devices](io_service_t service) {
        auto *d = createDeviceInfo(service);
        if (d) {
            devices.emplaceBack(d);
        }
    });

    return devices;
}

QObject *createDeviceMonitor(QObject *parent) {
    return new IOKitMonitor(parent);
}
