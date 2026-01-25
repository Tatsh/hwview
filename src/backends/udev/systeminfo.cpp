#include "systeminfo.h"
#include "driverinfo.h"
#include "udevdeviceinfo.h"
#include "udevmanager.h"
#include "udevmonitor.h"

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>
#include <QUrl>

#include <fcntl.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace {

// HID bus type constants from Linux input.h
constexpr int BUS_USB = 0x03;
constexpr int BUS_I2C = 0x18;

struct HidDeviceId {
    int busType = 0;
    QString vendorId;
    QString productId;
    QString instance;
    bool valid = false;
};

HidDeviceId parseHidDeviceId(const QString &path) {
    HidDeviceId result;
    static const QRegularExpression hidIdRe(
        QStringLiteral("([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4}):([0-9A-Fa-f]{4})\\.([0-9A-Fa-f]+)"));

    for (const QString &component : path.split(QLatin1Char('/'))) {
        auto match = hidIdRe.match(component);
        if (match.hasMatch()) {
            bool ok;
            result.busType = match.captured(1).toInt(&ok, 16);
            result.vendorId = match.captured(2);
            result.productId = match.captured(3);
            result.instance = match.captured(4);
            result.valid = ok;
            return result;
        }
    }
    return result;
}

QString hidBusTypeName(int busType) {
    switch (busType) {
    case 0x01:
        return QStringLiteral("PCI");
    case 0x03:
        return QStringLiteral("USB");
    case 0x05:
        return QStringLiteral("Bluetooth");
    case 0x06:
        return QStringLiteral("Virtual");
    case 0x18:
        return QStringLiteral("I²C");
    case 0x19:
        return QStringLiteral("Host");
    default:
        return {};
    }
}

int parseI2cBusNumber(const QString &path) {
    static const QRegularExpression i2cBusRe(QStringLiteral("/i2c-(\\d+)(?:/|$)"));
    auto match = i2cBusRe.match(path);
    if (match.hasMatch()) {
        bool ok;
        int bus = match.captured(1).toInt(&ok);
        if (ok) {
            return bus;
        }
    }
    return -1;
}

} // namespace

bool isComputerEntry(const QString &syspath) {
    return syspath == QStringLiteral("/sys/devices/virtual/dmi/id");
}

QString getComputerDisplayName() {
    QFileInfo acpiInfo(QStringLiteral("/sys/firmware/acpi"));
    QFileInfo dtInfo(QStringLiteral("/sys/firmware/devicetree"));

    if (acpiInfo.exists() && acpiInfo.isDir()) {
#if defined(Q_PROCESSOR_X86_64)
        return QObject::tr("ACPI x64-based PC");
#elif defined(Q_PROCESSOR_X86_32)
        return QObject::tr("ACPI x86-based PC");
#elif defined(Q_PROCESSOR_ARM_64)
        return QObject::tr("ACPI ARM64-based PC");
#elif defined(Q_PROCESSOR_ARM)
        return QObject::tr("ACPI ARM-based PC");
#else
        return QObject::tr("ACPI-based PC");
#endif
    }

    if (dtInfo.exists() && dtInfo.isDir()) {
        return QObject::tr("Device Tree-based System");
    }

    return QObject::tr("Standard PC");
}

QString getComputerSyspath() {
    return QStringLiteral("/sys/devices/virtual/dmi/id");
}

void openPrintersSettings() {
    // Linux/KDE: Open System Settings printers page
    QProcess::startDetached(QStringLiteral("systemsettings"),
                            {QStringLiteral("kcm_printer_manager")});
}

BuiltinDriverInfo getBuiltinDriverInfo() {
    BuiltinDriverInfo info;
    info.provider = QStringLiteral("Linux Foundation");
    info.copyright = QObject::tr("GPL-compatible");
    info.signer = QStringLiteral("Linux Foundation");
    info.builtinMessage = QObject::tr("(Built-in kernel module)");

    struct utsname buffer;
    if (uname(&buffer) == 0) {
        info.version = QString::fromLocal8Bit(buffer.release);
    }

    return info;
}

DriverFileDetails getDriverFileDetails(const QString &driverPath, const QString &driverName) {
    Q_UNUSED(driverName)
    DriverFileDetails details;

    // Check if this is an nvidia module
    QString moduleName = driverPath.section(QLatin1Char('/'), -1);
    bool isNvidia = moduleName.startsWith(QStringLiteral("nvidia"));

    // Get driver info from the existing function
    DriverInfo info = getDriverInfo(driverPath);

    if (isNvidia) {
        details.provider = QStringLiteral("NVIDIA Corporation");
        details.copyright = QStringLiteral("NVIDIA Driver License Agreement");
        details.signer = QStringLiteral("NVIDIA Corporation");
    } else {
        details.provider = info.author.isEmpty() ? QStringLiteral("Linux Foundation") : info.author;
        details.copyright = info.license;
        details.signer = info.signer.isEmpty() ? QStringLiteral("Linux Foundation") : info.signer;
    }

    if (!info.version.isEmpty()) {
        details.version = info.version;
    } else {
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            details.version = QString::fromLocal8Bit(buffer.release);
        }
    }

    return details;
}

QString formatDriverPath(const QString &path) {
    return path;
}

QString getDeviceDisplayName(const DeviceInfo &info) {
    QString name = info.name();
    QString subsystem = info.subsystem();

    // For storage volumes, try partition label or filesystem label
    if (subsystem == QStringLiteral("block")) {
        QString devtype = info.propertyValue("DEVTYPE");
        if (devtype == QStringLiteral("partition")) {
            QString partName = info.propertyValue("ID_PART_ENTRY_NAME");
            if (partName.isEmpty()) {
                partName = info.propertyValue("ID_FS_LABEL");
            }
            if (!partName.isEmpty()) {
                return partName;
            }
        }
    }

    // Strip /dev/ prefix for display
    QString shortName = name;
    if (shortName.startsWith(QStringLiteral("/dev/"))) {
        shortName = shortName.mid(5);
    }

    // Handle input/event* devices
    if (shortName.startsWith(QStringLiteral("input/event"))) {
        QString num = shortName.mid(11);
        return QObject::tr("Input event %1").arg(num);
    }
    // Handle input/mouse* devices
    if (shortName.startsWith(QStringLiteral("input/mouse"))) {
        QString num = shortName.mid(11);
        return QObject::tr("Input mouse %1").arg(num);
    }

    // Software/misc device friendly names
    if (subsystem == QStringLiteral("misc") || subsystem == QStringLiteral("input")) {
        static const QHash<QString, QString> softwareDeviceNames = {
            {QStringLiteral("autofs"), QObject::tr("Automount filesystem")},
            {QStringLiteral("cpu_dma_latency"), QObject::tr("CPU DMA latency")},
            {QStringLiteral("fuse"), QObject::tr("FUSE interface")},
            {QStringLiteral("hpet"), QObject::tr("High Precision Event Timer")},
            {QStringLiteral("hwrng"), QObject::tr("Hardware random number generator")},
            {QStringLiteral("kvm"), QObject::tr("Kernel-based Virtual Machine")},
            {QStringLiteral("loop-control"), QObject::tr("Loop device control")},
            {QStringLiteral("mcelog"), QObject::tr("Machine check error log")},
            {QStringLiteral("net/tun"), QObject::tr("TUN/TAP network device")},
            {QStringLiteral("ntsync"), QObject::tr("NT synchronization")},
            {QStringLiteral("rfkill"), QObject::tr("RF kill switch")},
            {QStringLiteral("uhid"), QObject::tr("User-space HID driver")},
            {QStringLiteral("uinput"), QObject::tr("User-space input device")},
            {QStringLiteral("vga_arbiter"), QObject::tr("VGA arbiter")},
            {QStringLiteral("vhost-net"), QObject::tr("VirtIO host network")},
            {QStringLiteral("mapper/control"), QObject::tr("Device mapper control")},
        };

        auto it = softwareDeviceNames.find(shortName);
        if (it != softwareDeviceNames.end()) {
            return it.value();
        }
    }

    // HID device friendly names based on device ID
    if (subsystem == QStringLiteral("hid")) {
        auto hidId = parseHidDeviceId(info.syspath());
        if (hidId.valid) {
            QString busName = hidBusTypeName(hidId.busType);
            if (!busName.isEmpty()) {
                return QObject::tr("%1 HID device").arg(busName);
            }
            return QObject::tr("HID device");
        }
    }

    // Battery/ACPI device names
    if (info.category() == DeviceCategory::Batteries) {
        QString devPath = info.propertyValue("DEVPATH");
        if (devPath.contains(QStringLiteral("PNP0C0A")) ||
            devPath.contains(QStringLiteral("battery"))) {
            return QObject::tr("Microsoft ACPI-Compliant Control Method Battery");
        }
        if (devPath.contains(QStringLiteral("AC")) || devPath.contains(QStringLiteral("ADP"))) {
            return QObject::tr("Microsoft AC Adapter");
        }
    }

    return name;
}

bool hasDriverInfo(const DeviceInfo &info) {
    // If device has an explicit driver, it has driver info
    if (!info.driver().isEmpty()) {
        return true;
    }

    QString subsystem = info.subsystem();
    QString devName = info.name();
    QString devNode = info.devnode();
    QString shortName = devName.startsWith(QStringLiteral("/dev/")) ? devName.mid(5) : devName;
    QString shortNode = devNode.startsWith(QStringLiteral("/dev/")) ? devNode.mid(5) : devNode;

    // VirtualBox devices are not Linux Foundation
    if (shortName.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive) ||
        shortNode.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive)) {
        return false;
    }

    // Misc and input devices are Linux Foundation devices
    if (subsystem == QStringLiteral("misc") || subsystem == QStringLiteral("input")) {
        return true;
    }

    // Storage volume partitions are Linux Foundation devices
    if (subsystem == QStringLiteral("block")) {
        QString devtype = info.propertyValue("DEVTYPE");
        if (devtype == QStringLiteral("partition")) {
            return true;
        }
    }

    return false;
}

static QString safeReadSysfsFile(const QString &path) {
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

QString getKernelVersion() {
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        return QString::fromLocal8Bit(buffer.release);
    }
    return {};
}

QString getKernelBuildDate() {
    QFile versionFile(QStringLiteral("/proc/version"));
    if (versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(versionFile.readAll());
        versionFile.close();

        // Parse build date from /proc/version
        // Format: "Linux version X.X.X (user@host) (gcc ...) #N SMP ... Wed Jan 15 17:42:44 UTC
        // 2025"
        QRegularExpression dateRe(QStringLiteral("(Mon|Tue|Wed|Thu|Fri|Sat|Sun)\\s+"
                                                 "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)"
                                                 "\\s+"
                                                 "(\\d{1,2})\\s+"
                                                 "(\\d{2}):(\\d{2}):(\\d{2})\\s+"
                                                 "(?:\\w+\\s+)?" // Optional timezone
                                                 "(\\d{4})"));

        QRegularExpressionMatch match = dateRe.match(content);
        if (match.hasMatch()) {
            QString monthStr = match.captured(2);
            int day = match.captured(3).toInt();
            int year = match.captured(7).toInt();

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
            int month = static_cast<int>(months.indexOf(monthStr)) + 1;

            if (month > 0) {
                QDate date(year, month, day);
                if (date.isValid()) {
                    return QLocale().toString(date, QLocale::ShortFormat);
                }
            }
        }

        // Fallback: return version string
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return QString::fromLocal8Bit(buffer.version);
        }
    }
    return {};
}

QString translateDevicePath(const QString &devpath) {
    if (devpath.isEmpty()) {
        return {};
    }

    // Check for PCI device: extract bus, device, function
    static const QRegularExpression pciRe(
        QStringLiteral("([0-9a-fA-F]{4}):([0-9a-fA-F]{2}):([0-9a-fA-F]{2})\\.([0-9a-fA-F])"));

    QRegularExpressionMatchIterator it = pciRe.globalMatch(devpath);
    int pciBus = -1, pciDevice = -1, pciFunction = -1;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        bool ok;
        pciBus = match.captured(2).toInt(&ok, 16);
        pciDevice = match.captured(3).toInt(&ok, 16);
        pciFunction = match.captured(4).toInt(&ok, 16);
    }

    // Check for HID device ID (format: XXXX:VVVV:PPPP.IIII)
    auto hidId = parseHidDeviceId(devpath);
    if (hidId.valid) {
        QString busName = hidBusTypeName(hidId.busType);
        int i2cBus = parseI2cBusNumber(devpath);

        if (hidId.busType == BUS_I2C && i2cBus >= 0) {
            if (pciBus >= 0 && pciDevice >= 0 && pciFunction >= 0) {
                return QObject::tr("On I²C bus %1 at PCI bus %2, device %3, function %4")
                    .arg(i2cBus)
                    .arg(pciBus)
                    .arg(pciDevice)
                    .arg(pciFunction);
            }
            return QObject::tr("On I²C bus %1").arg(i2cBus);
        } else if (hidId.busType == BUS_USB) {
            static const QRegularExpression usbReHid(QStringLiteral("/usb(\\d+)/(\\d+)-([\\d.]+)"));
            auto usbMatch = usbReHid.match(devpath);
            if (usbMatch.hasMatch()) {
                return QObject::tr("On USB bus %1, port %2")
                    .arg(usbMatch.captured(1), usbMatch.captured(3));
            }
            return QObject::tr("On USB bus");
        } else if (!busName.isEmpty()) {
            return QObject::tr("On %1 bus").arg(busName);
        }
    }

    // Check for USB device
    static const QRegularExpression usbRe(QStringLiteral("/usb(\\d+)/(\\d+)-([\\d.]+)"));
    auto usbMatch = usbRe.match(devpath);
    if (usbMatch.hasMatch()) {
        QString usbBus = usbMatch.captured(1);
        QString usbPort = usbMatch.captured(3);
        return QObject::tr("On USB bus %1, port %2").arg(usbBus, usbPort);
    }

    // Check for I²C device without HID ID
    int i2cBus = parseI2cBusNumber(devpath);
    if (i2cBus >= 0) {
        if (pciBus >= 0 && pciDevice >= 0 && pciFunction >= 0) {
            return QObject::tr("On I²C bus %1 at PCI bus %2, device %3, function %4")
                .arg(i2cBus)
                .arg(pciBus)
                .arg(pciDevice)
                .arg(pciFunction);
        }
        return QObject::tr("On I²C bus %1").arg(i2cBus);
    }

    // Check for SCSI device
    static const QRegularExpression scsiRe(
        QStringLiteral("/host(\\d+)/target\\d+:(\\d+):(\\d+)/(\\d+):(\\d+):(\\d+):(\\d+)"));
    auto scsiMatch = scsiRe.match(devpath);
    if (scsiMatch.hasMatch()) {
        QString busNum = scsiMatch.captured(5);
        QString targetId = scsiMatch.captured(6);
        QString lun = scsiMatch.captured(7);
        return QObject::tr("Bus number %1, target ID %2, LUN %3").arg(busNum, targetId, lun);
    }

    // Check for ACPI device
    static const QRegularExpression acpiPnpRe(
        QStringLiteral("/(PNP[0-9A-Fa-f]{4}|LNX[A-Z]+|ACPI[0-9A-Fa-f]{4}):([0-9]+)"));
    auto acpiMatch = acpiPnpRe.match(devpath);
    if (acpiMatch.hasMatch()) {
        QString pnpId = acpiMatch.captured(1);
        if (pnpId == QStringLiteral("PNP0C0A") || pnpId == QStringLiteral("ACPI0003")) {
            return QObject::tr("On ACPI-compliant system");
        }
        if (pnpId == QStringLiteral("PNP0C50")) {
            if (i2cBus >= 0) {
                return QObject::tr("On I2C HID bus %1").arg(i2cBus);
            }
            return QObject::tr("On I2C HID bus");
        }
        return QObject::tr("On ACPI-compliant system");
    }

    if (devpath.contains(QStringLiteral("/ACPI")) || devpath.contains(QStringLiteral("/acpi")) ||
        devpath.contains(QStringLiteral("/LNXSYSTM")) || devpath.contains(QStringLiteral("/PNP"))) {
        return QObject::tr("On ACPI-compliant system");
    }

    // Check for platform device
    if (devpath.contains(QStringLiteral("/platform/"))) {
        return QObject::tr("On system board");
    }

    // Check for virtual device
    if (devpath.contains(QStringLiteral("/virtual/"))) {
        return QObject::tr("Virtual device");
    }

    // If we found a PCI address, format it
    if (pciBus >= 0 && pciDevice >= 0 && pciFunction >= 0) {
        return QObject::tr("PCI bus %1, device %2, function %3")
            .arg(pciBus)
            .arg(pciDevice)
            .arg(pciFunction);
    }

    // Check for PS/2 devices
    if (devpath.contains(QStringLiteral("/i8042/"))) {
        return QObject::tr("Connected to PS/2 port");
    }

    // Check for input devices
    if (devpath.contains(QStringLiteral("/input/"))) {
        if (devpath.contains(QStringLiteral("/serio"))) {
            return QObject::tr("Connected to PS/2 port");
        }
        return QObject::tr("On input device");
    }

    return {};
}

QString getMountPoint(const QString &devnode) {
    if (devnode.isEmpty()) {
        return {};
    }

    QFileInfo devnodeInfo(devnode);
    QString canonicalDevnode = devnodeInfo.canonicalFilePath();
    if (canonicalDevnode.isEmpty()) {
        canonicalDevnode = devnode;
    }

    QFile mountsFile(QStringLiteral("/proc/mounts"));
    if (!mountsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QByteArray allData = mountsFile.readAll();
    mountsFile.close();

    QStringList mountLines =
        QString::fromUtf8(allData).split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString &line : mountLines) {
        QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            QString device = parts.at(0);
            QString mountPoint = parts.at(1);

            QFileInfo mountDevInfo(device);
            QString canonicalMountDev = mountDevInfo.canonicalFilePath();
            if (canonicalMountDev.isEmpty()) {
                canonicalMountDev = device;
            }

            if (device == devnode || device == canonicalDevnode || canonicalMountDev == devnode ||
                canonicalMountDev == canonicalDevnode) {
                // Decode escaped characters
                mountPoint.replace(QStringLiteral("\\040"), QStringLiteral(" "));
                mountPoint.replace(QStringLiteral("\\011"), QStringLiteral("\t"));
                mountPoint.replace(QStringLiteral("\\012"), QStringLiteral("\n"));
                mountPoint.replace(QStringLiteral("\\134"), QStringLiteral("\\"));
                return mountPoint;
            }
        }
    }

    return {};
}

QString lookupUsbVendor(const QString &vendorId) {
    static QHash<QString, QString> vendorCache;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        static const QStringList usbIdsLocations = {
            QStringLiteral("/usr/share/hwdata/usb.ids"),
            QStringLiteral("/usr/share/misc/usb.ids"),
            QStringLiteral("/usr/share/usb.ids"),
            QStringLiteral("/var/lib/usbutils/usb.ids"),
        };

        for (const QString &path : usbIdsLocations) {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }

            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.length() >= 6 && !line.startsWith(QLatin1Char('\t')) &&
                    !line.startsWith(QLatin1Char('#'))) {
                    QString lineVendorId = line.left(4).toLower();
                    QString name = line.mid(6).trimmed();
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
    query.vendorId = info.propertyValue("ID_VENDOR_ID");
    query.modelId = info.propertyValue("ID_MODEL_ID");
    return query;
}

QStringList queryDeviceEvents(const DeviceEventQuery &query) {
    QStringList events;
    QStringList searchTerms;

    if (!query.vendorId.isEmpty() && !query.modelId.isEmpty()) {
        searchTerms << QStringLiteral("idVendor=%1").arg(query.vendorId.toLower());
    }

    if (!query.deviceName.isEmpty() && query.deviceName.length() >= 8) {
        QString nameSearch = query.deviceName.left(20).trimmed();
        auto lastSpace = nameSearch.lastIndexOf(QLatin1Char(' '));
        if (lastSpace > 8) {
            nameSearch = nameSearch.left(lastSpace);
        }
        searchTerms << nameSearch;
    }

    if (!query.devnode.isEmpty()) {
        QString shortName = query.devnode;
        if (shortName.startsWith(QStringLiteral("/dev/"))) {
            shortName = shortName.mid(5);
        }
        if (shortName.length() >= 3) {
            searchTerms << shortName;
        }
    }

    QRegularExpression pciRe(QStringLiteral("([0-9a-f]{4}:[0-9a-f]{2}:[0-9a-f]{2}\\.[0-9a-f])"));
    QRegularExpressionMatch pciMatch = pciRe.match(query.syspath);
    if (pciMatch.hasMatch()) {
        searchTerms << pciMatch.captured(1);
    }

    if (searchTerms.isEmpty()) {
        return events;
    }

    QProcess journalctl;
    QStringList args;
    args << QStringLiteral("-k") << QStringLiteral("-n") << QStringLiteral("500")
         << QStringLiteral("--no-pager") << QStringLiteral("-o") << QStringLiteral("short-iso");

    journalctl.start(QStringLiteral("journalctl"), args);
    if (journalctl.waitForFinished(5000)) {
        QString output = QString::fromUtf8(journalctl.readAllStandardOutput());
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

    // Parse journalctl output with -o short-iso format
    // Format: "YYYY-MM-DDTHH:MM:SS+ZZZZ hostname kernel: message"
    QRegularExpression isoTimestampRe(
        QStringLiteral("^(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}[+-]\\d{2}:?\\d{2})\\s+"));
    QRegularExpressionMatch match = isoTimestampRe.match(line);

    if (match.hasMatch()) {
        QString isoTimestamp = match.captured(1);
        QString remainder = line.mid(match.capturedEnd());

        QDateTime dateTime = QDateTime::fromString(isoTimestamp, Qt::ISODate);
        if (dateTime.isValid()) {
            result.timestamp = QLocale::system().toString(dateTime, QLocale::ShortFormat);
        } else {
            result.timestamp = isoTimestamp;
        }

        auto kernelIdx = remainder.indexOf(QStringLiteral("kernel:"));
        if (kernelIdx != -1) {
            result.message = remainder.mid(kernelIdx + 7).trimmed();
        } else {
            auto colonIdx = remainder.indexOf(QStringLiteral(": "));
            if (colonIdx != -1) {
                result.message = remainder.mid(colonIdx + 2).trimmed();
            } else {
                result.message = remainder.trimmed();
            }
        }
    } else {
        result.message = line;
    }

    return result;
}

static QString getBlockDeviceManufacturer(const QString &syspath,
                                          const QString &deviceName,
                                          const QString &parentSyspath,
                                          const QHash<QString, QString> &properties) {
    auto extractManufacturer = [](const QString &text) -> QString {
        if (text.isEmpty()) {
            return {};
        }

        static const QList<QPair<QString, QString>> manufacturerMappings = {
            {QStringLiteral("Samsung"), QStringLiteral("Samsung")},
            {QStringLiteral("WDC"), QStringLiteral("Western Digital")},
            {QStringLiteral("Western Digital"), QStringLiteral("Western Digital")},
            {QStringLiteral("Seagate"), QStringLiteral("Seagate")},
            {QStringLiteral("Toshiba"), QStringLiteral("Toshiba")},
            {QStringLiteral("HGST"), QStringLiteral("HGST")},
            {QStringLiteral("Hitachi"), QStringLiteral("Hitachi")},
            {QStringLiteral("Kingston"), QStringLiteral("Kingston")},
            {QStringLiteral("SanDisk"), QStringLiteral("SanDisk")},
            {QStringLiteral("Crucial"), QStringLiteral("Crucial")},
            {QStringLiteral("CT"), QStringLiteral("Crucial")},
            {QStringLiteral("Intel"), QStringLiteral("Intel")},
            {QStringLiteral("Micron"), QStringLiteral("Micron")},
            {QStringLiteral("SK hynix"), QStringLiteral("SK hynix")},
            {QStringLiteral("KIOXIA"), QStringLiteral("KIOXIA")},
            {QStringLiteral("Phison"), QStringLiteral("Phison")},
            {QStringLiteral("Realtek"), QStringLiteral("Realtek")},
            {QStringLiteral("Sabrent"), QStringLiteral("Sabrent")},
            {QStringLiteral("ADATA"), QStringLiteral("ADATA")},
            {QStringLiteral("PNY"), QStringLiteral("PNY")},
            {QStringLiteral("Corsair"), QStringLiteral("Corsair")},
            {QStringLiteral("Transcend"), QStringLiteral("Transcend")},
            {QStringLiteral("LiteOn"), QStringLiteral("Lite-On")},
            {QStringLiteral("LITE-ON"), QStringLiteral("Lite-On")},
            {QStringLiteral("Plextor"), QStringLiteral("Plextor")},
            {QStringLiteral("OCZ"), QStringLiteral("OCZ")},
            {QStringLiteral("Patriot"), QStringLiteral("Patriot")},
            {QStringLiteral("SPCC"), QStringLiteral("Silicon Power")},
            {QStringLiteral("Silicon Power"), QStringLiteral("Silicon Power")},
            {QStringLiteral("Team"), QStringLiteral("Team Group")},
            {QStringLiteral("Lexar"), QStringLiteral("Lexar")},
            {QStringLiteral("HP"), QStringLiteral("HP")},
            {QStringLiteral("Dell"), QStringLiteral("Dell")},
            {QStringLiteral("Lenovo"), QStringLiteral("Lenovo")},
            {QStringLiteral("Apple"), QStringLiteral("Apple")},
            {QStringLiteral("Maxtor"), QStringLiteral("Maxtor")},
            {QStringLiteral("Fujitsu"), QStringLiteral("Fujitsu")},
        };

        for (const auto &mapping : manufacturerMappings) {
            if (text.startsWith(mapping.first, Qt::CaseInsensitive)) {
                return mapping.second;
            }
            if (text.contains(mapping.first, Qt::CaseInsensitive)) {
                return mapping.second;
            }
        }

        return {};
    };

    // Try device name first
    QString manufacturer = extractManufacturer(deviceName);
    if (!manufacturer.isEmpty()) {
        return manufacturer;
    }

    // Try ID_MODEL property
    QString model = properties.value(QStringLiteral("ID_MODEL"));
    manufacturer = extractManufacturer(model);
    if (!manufacturer.isEmpty()) {
        return manufacturer;
    }

    // Try ID_MODEL_FROM_DATABASE
    model = properties.value(QStringLiteral("ID_MODEL_FROM_DATABASE"));
    manufacturer = extractManufacturer(model);
    if (!manufacturer.isEmpty()) {
        return manufacturer;
    }

    // Try to read vendor from sysfs
    QString vendorPath = syspath + QStringLiteral("/device/vendor");
    QString vendor = safeReadSysfsFile(vendorPath);
    if (!vendor.isEmpty()) {
        vendor = vendor.trimmed();

        if (vendor == QStringLiteral("ATA") || vendor == QStringLiteral("SATA") ||
            vendor == QStringLiteral("USB") || vendor == QStringLiteral("Generic") ||
            vendor == QStringLiteral("NVMe")) {
            // Skip generic strings
        } else {
            static const QHash<QString, QString> vendorMappings = {
                {QStringLiteral("SAMSUNG"), QStringLiteral("Samsung")},
                {QStringLiteral("WDC"), QStringLiteral("Western Digital")},
                {QStringLiteral("SEAGATE"), QStringLiteral("Seagate")},
                {QStringLiteral("TOSHIBA"), QStringLiteral("Toshiba")},
                {QStringLiteral("HITACHI"), QStringLiteral("Hitachi")},
                {QStringLiteral("HGST"), QStringLiteral("HGST")},
                {QStringLiteral("HL-DT-ST"), QStringLiteral("LG Electronics")},
                {QStringLiteral("HLDS"), QStringLiteral("LG Electronics")},
                {QStringLiteral("TSSTcorp"), QStringLiteral("Toshiba Samsung Storage Technology")},
                {QStringLiteral("PIONEER"), QStringLiteral("Pioneer")},
                {QStringLiteral("ASUS"), QStringLiteral("ASUSTeK Computer")},
                {QStringLiteral("LITE-ON"), QStringLiteral("Lite-On")},
                {QStringLiteral("LITEON"), QStringLiteral("Lite-On")},
                {QStringLiteral("MATSHITA"), QStringLiteral("Panasonic")},
                {QStringLiteral("PANASONIC"), QStringLiteral("Panasonic")},
                {QStringLiteral("SONY"), QStringLiteral("Sony")},
                {QStringLiteral("NEC"), QStringLiteral("NEC")},
                {QStringLiteral("PLEXTOR"), QStringLiteral("Plextor")},
                {QStringLiteral("BENQ"), QStringLiteral("BenQ")},
                {QStringLiteral("OPTIARC"), QStringLiteral("Sony NEC Optiarc")},
                {QStringLiteral("TEAC"), QStringLiteral("TEAC")},
            };

            auto it = vendorMappings.find(vendor.toUpper());
            if (it != vendorMappings.end()) {
                return it.value();
            }

            if (vendor.startsWith(QStringLiteral("WD"), Qt::CaseInsensitive)) {
                return QStringLiteral("Western Digital");
            }

            return vendor;
        }
    }

    // Try parent device's vendor
    if (!parentSyspath.isEmpty()) {
        QString parentVendorPath = parentSyspath + QStringLiteral("/vendor");
        vendor = safeReadSysfsFile(parentVendorPath);
        if (!vendor.isEmpty()) {
            return vendor.trimmed();
        }
    }

    return {};
}

namespace {
bool isPciDevice(const QString &syspath) {
    static const QRegularExpression pciPathRe(
        QStringLiteral("/pci[^/]*/[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\\.[0-9a-fA-F]$"));
    return pciPathRe.match(syspath).hasMatch();
}
} // namespace

QList<ResourceInfo> getDeviceResources(const QString &syspath, const QString &driver) {
    QList<ResourceInfo> resources;

    if (!isPciDevice(syspath)) {
        return resources;
    }

    // Get IRQ
    QString irqPath = syspath + QStringLiteral("/irq");
    QString irq = safeReadSysfsFile(irqPath);
    if (!irq.isEmpty() && irq != QStringLiteral("0")) {
        int irqNum = irq.toInt();
        QString setting =
            QStringLiteral("0x%1 (%2)").arg(irqNum, 8, 16, QLatin1Char('0')).arg(irqNum).toUpper();
        resources.append({QObject::tr("IRQ"), setting, QStringLiteral("preferences-other")});
    }

    // Get PCI resources
    QString resourcePath = syspath + QStringLiteral("/resource");
    QString resourceContent = safeReadSysfsFile(resourcePath);
    if (!resourceContent.isEmpty()) {
        static const QRegularExpression whitespaceRe(QStringLiteral("\\s+"));
        const QStringList lines = resourceContent.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            QString trimmedLine = line.trimmed();
            if (trimmedLine.isEmpty()) {
                continue;
            }

            QStringList parts = trimmedLine.split(whitespaceRe, Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                bool ok1, ok2, ok3;
                qulonglong start = parts[0].toULongLong(&ok1, 16);
                qulonglong end = parts[1].toULongLong(&ok2, 16);
                qulonglong flags = parts[2].toULongLong(&ok3, 16);

                if (ok1 && ok2 && ok3 && start != 0 && end != 0) {
                    QString type;
                    if (flags & 0x00000100) {
                        type = QObject::tr("I/O Range");
                    } else if (flags & 0x00000200) {
                        type = QObject::tr("Memory Range");
                    } else {
                        continue;
                    }

                    QString setting = QStringLiteral("%1 - %2")
                                          .arg(start, 16, 16, QLatin1Char('0'))
                                          .arg(end, 16, 16, QLatin1Char('0'))
                                          .toUpper();
                    resources.append({type, setting, QStringLiteral("drive-harddisk")});
                }
            }
        }
    }

    // Get DMA
    if (!driver.isEmpty()) {
        QString dmaContent = safeReadSysfsFile(QStringLiteral("/proc/dma"));
        if (!dmaContent.isEmpty()) {
            static const QRegularExpression dmaRe(QStringLiteral("^(\\d+):\\s*(.*)$"));
            const QStringList lines = dmaContent.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                if (line.contains(driver, Qt::CaseInsensitive)) {
                    auto match = dmaRe.match(line);
                    if (match.hasMatch()) {
                        QString channel = match.captured(1);
                        resources.append(
                            {QObject::tr("DMA"), channel, QStringLiteral("preferences-other")});
                    }
                }
            }
        }
    }

    return resources;
}

QList<DmaChannelInfo> getSystemDmaChannels() {
    QList<DmaChannelInfo> channels;

    QString content = safeReadSysfsFile(QStringLiteral("/proc/dma"));
    if (content.isEmpty()) {
        return channels;
    }

    static const QRegularExpression dmaRe(QStringLiteral("^(\\d+):\\s*(.*)$"));
    const QStringList lines = content.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        auto match = dmaRe.match(trimmed);
        if (match.hasMatch()) {
            channels.append({match.captured(1), match.captured(2)});
        }
    }

    return channels;
}

QList<IoPortInfo> getSystemIoPorts() {
    QList<IoPortInfo> ports;

    QString content = safeReadSysfsFile(QStringLiteral("/proc/ioports"));
    if (content.isEmpty()) {
        return ports;
    }

    static const QRegularExpression resourceRe(
        QStringLiteral("^(\\s*)([0-9a-fA-F]+)-([0-9a-fA-F]+)\\s*:\\s*(.*)$"));

    const QStringList lines = content.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        auto match = resourceRe.match(line);
        if (!match.hasMatch()) {
            continue;
        }

        QString name = match.captured(4);
        if (name.isEmpty()) {
            continue;
        }

        IoPortInfo info;
        info.indentLevel = static_cast<int>(match.captured(1).length());
        info.rangeStart = match.captured(2).toUpper();
        info.rangeEnd = match.captured(3).toUpper();
        info.name = name;
        ports.append(info);
    }

    return ports;
}

QList<IrqInfo> getSystemIrqs() {
    QList<IrqInfo> irqs;

    QString content = safeReadSysfsFile(QStringLiteral("/proc/interrupts"));
    if (content.isEmpty()) {
        return irqs;
    }

    QStringList lines = content.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    if (!lines.isEmpty()) {
        lines.removeFirst(); // Skip header
    }

    static const QRegularExpression whitespaceRe(QStringLiteral("\\s+"));
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        auto parts = line.split(whitespaceRe, Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        QString irqNum = parts[0];
        if (irqNum.endsWith(QLatin1Char(':'))) {
            irqNum.chop(1);
        }

        QString deviceName;
        QString irqType;

        for (int i = 1; i < parts.size(); ++i) {
            if (parts[i].contains(QStringLiteral("APIC")) ||
                parts[i].contains(QStringLiteral("PCI")) ||
                parts[i].contains(QStringLiteral("MSI")) ||
                parts[i].contains(QStringLiteral("DMAR")) ||
                parts[i].contains(QStringLiteral("edge")) ||
                parts[i].contains(QStringLiteral("level")) ||
                parts[i].contains(QStringLiteral("fasteoi"))) {
                if (irqType.isEmpty()) {
                    irqType = parts[i];
                } else {
                    irqType += QLatin1Char(' ') + parts[i];
                }
            } else if (i > 1 && !parts[i].isEmpty() && !parts[i].at(0).isDigit()) {
                if (deviceName.isEmpty()) {
                    deviceName = parts[i];
                } else {
                    deviceName += QLatin1Char(' ') + parts[i];
                }
            }
        }

        if (deviceName.isEmpty()) {
            continue;
        }

        irqs.append({irqNum, irqType, deviceName});
    }

    return irqs;
}

QList<MemoryRangeInfo> getSystemMemoryRanges() {
    QList<MemoryRangeInfo> ranges;

    QString content = safeReadSysfsFile(QStringLiteral("/proc/iomem"));
    if (content.isEmpty()) {
        return ranges;
    }

    static const QRegularExpression resourceRe(
        QStringLiteral("^(\\s*)([0-9a-fA-F]+)-([0-9a-fA-F]+)\\s*:\\s*(.*)$"));

    const QStringList lines = content.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        auto match = resourceRe.match(line);
        if (!match.hasMatch()) {
            continue;
        }

        QString name = match.captured(4);
        if (name.isEmpty()) {
            continue;
        }

        MemoryRangeInfo info;
        info.indentLevel = static_cast<int>(match.captured(1).length());
        info.rangeStart = match.captured(2).toUpper();
        info.rangeEnd = match.captured(3).toUpper();
        info.name = name;
        ranges.append(info);
    }

    return ranges;
}

QList<PropertyMapping> getDevicePropertyMappings() {
    return {
        {QObject::tr("Device description"), PropertyKeys::deviceDescription(), false},
        {QObject::tr("Hardware IDs"), QStringLiteral("MODALIAS"), false},
        {QObject::tr("Compatible IDs"), PropertyKeys::compatibleIds(), true},
        {QObject::tr("Device class"), QStringLiteral("ID_PCI_CLASS_FROM_DATABASE"), false},
        {QObject::tr("Device class GUID"), QStringLiteral("ID_PCI_CLASS"), false},
        {QObject::tr("Driver key"), QStringLiteral("DRIVER"), false},
        {QObject::tr("Device instance path"), QStringLiteral("DEVPATH"), false},
        {QObject::tr("Location paths"), QStringLiteral("DEVPATH"), false},
        {QObject::tr("Physical device object name"), QStringLiteral("DEVNAME"), false},
        {QObject::tr("Bus relations"), QStringLiteral("SUBSYSTEM"), false},
        {QObject::tr("Parent"), PropertyKeys::parentSyspath(), false},
        {QObject::tr("Children"), PropertyKeys::children(), true},
        {QObject::tr("Manufacturer"), QStringLiteral("ID_VENDOR_FROM_DATABASE"), false},
        {QObject::tr("Model"), QStringLiteral("ID_MODEL_FROM_DATABASE"), false},
        {QObject::tr("Serial number"), QStringLiteral("ID_SERIAL"), false},
        {QObject::tr("Subsystem"), QStringLiteral("SUBSYSTEM"), false},
        {QObject::tr("Device node"), QStringLiteral("DEVNAME"), false},
        {QObject::tr("Syspath"), PropertyKeys::syspath(), false},
        {QObject::tr("Mount point"), PropertyKeys::mountPoint(), false},
    };
}

QStringList convertToHardwareIds(const QString &propertyKey, const QString &value) {
    QStringList result;

    if (propertyKey != QStringLiteral("MODALIAS") || value.isEmpty()) {
        return result;
    }

    // Parse PCI modalias: pci:vXXXXXXXXdXXXXXXXXsvXXXXXXXXsdXXXXXXXXbcXXscXXiXX
    static const QRegularExpression pciRe(QStringLiteral("^pci:v([0-9A-Fa-f]{8})d([0-9A-Fa-f]{8})"
                                                         "sv([0-9A-Fa-f]{8})sd([0-9A-Fa-f]{8})"));
    auto pciMatch = pciRe.match(value);
    if (pciMatch.hasMatch()) {
        QString vendorId = pciMatch.captured(1).right(4).toUpper();
        QString deviceId = pciMatch.captured(2).right(4).toUpper();
        QString subVendor = pciMatch.captured(3).right(4).toUpper();
        QString subDevice = pciMatch.captured(4).right(4).toUpper();
        QString subsys = subVendor + subDevice;
        result << QStringLiteral("PCI\\VEN_%1&DEV_%2&SUBSYS_%3").arg(vendorId, deviceId, subsys);
    }

    // Parse USB modalias: usb:vXXXXpXXXXdXXXX...
    static const QRegularExpression usbRe(
        QStringLiteral("^usb:v([0-9A-Fa-f]{4})p([0-9A-Fa-f]{4})"));
    auto usbMatch = usbRe.match(value);
    if (usbMatch.hasMatch()) {
        QString vendorId = usbMatch.captured(1).toUpper();
        QString productId = usbMatch.captured(2).toUpper();
        result << QStringLiteral("USB\\VID_%1&PID_%2").arg(vendorId, productId);
    }

    // Parse ACPI modalias: acpi:PNPXXXX: or acpi:ACPIXXXX:
    static const QRegularExpression acpiRe(QStringLiteral("^acpi:([A-Za-z0-9]+):"));
    auto acpiMatch = acpiRe.match(value);
    if (acpiMatch.hasMatch()) {
        QString acpiId = acpiMatch.captured(1).toUpper();
        result << QStringLiteral("ACPI\\%1").arg(acpiId);
    }

    // Parse HID modalias: hid:bXXXXgXXXXvXXXXXXXXpXXXXXXXX
    static const QRegularExpression hidRe(QStringLiteral("^hid:b([0-9A-Fa-f]{4})g[0-9A-Fa-f]{4}"
                                                         "v([0-9A-Fa-f]{8})p([0-9A-Fa-f]{8})"));
    auto hidMatch = hidRe.match(value);
    if (hidMatch.hasMatch()) {
        QString vendorId = hidMatch.captured(2).right(4).toUpper();
        QString productId = hidMatch.captured(3).right(4).toUpper();
        result << QStringLiteral("HID\\VID_%1&PID_%2").arg(vendorId, productId);
    }

    return result;
}

BasicDriverInfo getBasicDriverInfo(const QString &driver) {
    BasicDriverInfo info;
    info.provider = QStringLiteral("Linux Foundation");
    info.version = getKernelVersion();
    info.signer = QStringLiteral("Linux Foundation");
    info.date = getKernelBuildDate();
    info.hasDriverFiles = false;

    if (driver.isEmpty()) {
        return info;
    }

    QProcess modinfo;
    modinfo.start(QStringLiteral("modinfo"), {driver});
    if (!modinfo.waitForFinished(3000)) {
        return info;
    }

    QString output = QString::fromUtf8(modinfo.readAllStandardOutput());
    QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    QString filename, author, version, signer;

    for (const QString &line : lines) {
        auto colonIdx = line.indexOf(QLatin1Char(':'));
        if (colonIdx < 0) {
            continue;
        }

        QString key = line.left(colonIdx).trimmed();
        QString value = line.mid(colonIdx + 1).trimmed();

        if (key == QStringLiteral("filename")) {
            filename = value;
        } else if (key == QStringLiteral("author") && author.isEmpty()) {
            author = value;
        } else if (key == QStringLiteral("version")) {
            version = value;
        } else if (key == QStringLiteral("signer")) {
            signer = value;
        }
    }

    info.hasDriverFiles = !filename.isEmpty();

    bool isOutOfTree = !filename.isEmpty() && filename != QStringLiteral("(builtin)") &&
                       !filename.contains(QStringLiteral("/kernel/"));

    if (driver == QStringLiteral("nvidia") || driver.startsWith(QStringLiteral("nvidia_"))) {
        info.provider = QStringLiteral("NVIDIA Corporation");
        if (!version.isEmpty()) {
            info.version = version;
        }
        info.signer = QStringLiteral("NVIDIA Corporation");
    } else if (isOutOfTree) {
        if (!author.isEmpty()) {
            info.provider = author;
        }
        if (!version.isEmpty()) {
            info.version = version;
        }
        if (!signer.isEmpty()) {
            info.signer = signer;
        } else if (!author.isEmpty()) {
            info.signer = author;
        }
    } else {
        if (!version.isEmpty()) {
            info.version = version;
        }
        if (!signer.isEmpty()) {
            info.signer = signer;
        }
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
    // Storage volumes (partitions) don't have a manufacturer
    if (info.category() == DeviceCategory::StorageVolumes) {
        return {};
    }

    QString manufacturer = info.propertyValue("ID_VENDOR_FROM_DATABASE");

    // For block devices (disk drives, DVD/CD-ROM), try harder to get manufacturer
    if (manufacturer.isEmpty() && info.subsystem() == QStringLiteral("block")) {
        QHash<QString, QString> properties;
        properties.insert(QStringLiteral("ID_MODEL"), info.propertyValue("ID_MODEL"));
        properties.insert(QStringLiteral("ID_MODEL_FROM_DATABASE"),
                          info.propertyValue("ID_MODEL_FROM_DATABASE"));
        manufacturer = getBlockDeviceManufacturer(
            info.syspath(), info.name(), info.parentSyspath(), properties);
    }

    if (manufacturer.isEmpty()) {
        manufacturer = info.propertyValue("ID_USB_VENDOR");
    }
    if (manufacturer.isEmpty()) {
        manufacturer = info.propertyValue("ID_VENDOR");
    }
    if (manufacturer.isEmpty()) {
        QString encoded = info.propertyValue("ID_VENDOR_ENC");
        if (!encoded.isEmpty()) {
            manufacturer = QUrl::fromPercentEncoding(encoded.toUtf8());
            manufacturer.replace(QLatin1Char('_'), QLatin1Char(' '));
        }
    }
    if (manufacturer.isEmpty()) {
        QString syspath = info.syspath();
        QString vendorId;

        // Try uhid path: /devices/virtual/misc/uhid/xxxx:yyyy:zzzz
        static const QRegularExpression uhidRe(QStringLiteral(
            "/devices/virtual/misc/uhid/[0-9a-fA-F]{4}:([0-9a-fA-F]{4}):[0-9a-fA-F]{4}"));
        auto match = uhidRe.match(syspath);
        if (match.hasMatch()) {
            vendorId = match.captured(1).toLower();
        }

        // Try USB device path
        if (vendorId.isEmpty()) {
            static const QRegularExpression usbDevRe(
                QStringLiteral("[0-9a-fA-F]{4}:([0-9a-fA-F]{4}):[0-9a-fA-F]{4}\\.[0-9a-fA-F]{4}$"));
            match = usbDevRe.match(syspath);
            if (match.hasMatch()) {
                vendorId = match.captured(1).toLower();
            }
        }

        // Try HID device ID format
        if (vendorId.isEmpty()) {
            auto hidId = parseHidDeviceId(syspath);
            if (hidId.valid) {
                vendorId = hidId.vendorId.toLower();
            }
        }

        if (!vendorId.isEmpty()) {
            manufacturer = lookupUsbVendor(vendorId);
        }
    }

    // Fallback for known vendors
    if (manufacturer == QStringLiteral("046d")) {
        manufacturer = QStringLiteral("Logitech, Inc.");
    }
    // Clean up ugly manufacturer names
    if (manufacturer == QStringLiteral("Metadot_-_Das_Keyboard") ||
        manufacturer == QStringLiteral("Metadot - Das Keyboard")) {
        manufacturer = QStringLiteral("Metadot");
    }

    // Override manufacturer based on device name for known devices
    QString devName = info.name();
    QString devNode = info.devnode();
    QString shortName = devName.startsWith(QStringLiteral("/dev/")) ? devName.mid(5) : devName;
    QString shortNode = devNode.startsWith(QStringLiteral("/dev/")) ? devNode.mid(5) : devNode;

    // Oracle/VirtualBox devices
    if (shortName.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive) ||
        shortNode.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive)) {
        return QStringLiteral("Oracle Corporation");
    }
    // Intel Corporation devices
    if (shortName == QStringLiteral("i8042")) {
        return QStringLiteral("Intel Corporation");
    }
    // Linux Foundation devices (misc/software devices and input devices)
    if (shortName == QStringLiteral("autofs") || shortName == QStringLiteral("cpu dma latency") ||
        shortName == QStringLiteral("cpu_dma_latency") || shortName == QStringLiteral("fuse") ||
        shortName == QStringLiteral("hpet") || shortName == QStringLiteral("hwrng") ||
        shortName == QStringLiteral("kvm") || shortName == QStringLiteral("loop-control") ||
        shortName == QStringLiteral("loop control") || shortName == QStringLiteral("mcelog") ||
        shortName == QStringLiteral("net/tun") || shortName == QStringLiteral("ntsync") ||
        shortName == QStringLiteral("rfkill") || shortName == QStringLiteral("uhid") ||
        shortName == QStringLiteral("uinput") || shortName == QStringLiteral("vga arbiter") ||
        shortName == QStringLiteral("vhost-net") ||
        shortName.startsWith(QStringLiteral("input/event")) ||
        shortName.startsWith(QStringLiteral("input/mouse")) ||
        shortNode == QStringLiteral("autofs") || shortNode == QStringLiteral("cpu_dma_latency") ||
        shortNode == QStringLiteral("fuse") || shortNode == QStringLiteral("hpet") ||
        shortNode == QStringLiteral("hwrng") || shortNode == QStringLiteral("kvm") ||
        shortNode == QStringLiteral("loop-control") || shortNode == QStringLiteral("mcelog") ||
        shortNode == QStringLiteral("net/tun") || shortNode == QStringLiteral("ntsync") ||
        shortNode == QStringLiteral("rfkill") || shortNode == QStringLiteral("uhid") ||
        shortNode == QStringLiteral("uinput") ||
        shortNode.startsWith(QStringLiteral("input/event")) ||
        shortNode.startsWith(QStringLiteral("input/mouse")) ||
        shortNode == QStringLiteral("vga_arbiter") || shortNode == QStringLiteral("vhost-net") ||
        shortName == QStringLiteral("mapper/control") ||
        shortNode == QStringLiteral("mapper/control")) {
        return QStringLiteral("Linux Foundation");
    }

    return manufacturer;
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
    QHash<QString, QString> distro;

    QFile osRelease(QStringLiteral("/etc/os-release"));
    if (osRelease.open(QIODevice::ReadOnly | QIODevice::Text)) {
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
    }

    return distro;
}

QHash<QString, QString> getExportDeviceProperties(const DeviceInfo &info) {
    QHash<QString, QString> properties;

    if (info.syspath().isEmpty()) {
        return properties;
    }

    // Linux-specific property names
    auto addIfNotEmpty = [&](const QString &key, const char *propName) {
        QString value = info.propertyValue(propName);
        if (!value.isEmpty()) {
            properties[key] = value;
        }
    };

    addIfNotEmpty(QStringLiteral("ID_VENDOR_FROM_DATABASE"), "ID_VENDOR_FROM_DATABASE");
    addIfNotEmpty(QStringLiteral("ID_VENDOR"), "ID_VENDOR");
    addIfNotEmpty(QStringLiteral("ID_VENDOR_ENC"), "ID_VENDOR_ENC");
    addIfNotEmpty(QStringLiteral("ID_USB_VENDOR"), "ID_USB_VENDOR");
    addIfNotEmpty(QStringLiteral("ID_MODEL"), "ID_MODEL");
    addIfNotEmpty(QStringLiteral("ID_MODEL_FROM_DATABASE"), "ID_MODEL_FROM_DATABASE");
    addIfNotEmpty(QStringLiteral("ID_SERIAL"), "ID_SERIAL");
    addIfNotEmpty(QStringLiteral("MODALIAS"), "MODALIAS");
    addIfNotEmpty(QStringLiteral("DEVTYPE"), "DEVTYPE");
    addIfNotEmpty(QStringLiteral("ID_PART_ENTRY_NAME"), "ID_PART_ENTRY_NAME");
    addIfNotEmpty(QStringLiteral("ID_FS_LABEL"), "ID_FS_LABEL");
    addIfNotEmpty(QStringLiteral("ID_VENDOR_ID"), "ID_VENDOR_ID");
    addIfNotEmpty(QStringLiteral("ID_MODEL_ID"), "ID_MODEL_ID");

    return properties;
}

static bool isPciDeviceForExport(const QString &syspath) {
    static const QRegularExpression pciPathRe(
        QStringLiteral("/pci[^/]*/[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\\.[0-9a-fA-F]$"));
    return pciPathRe.match(syspath).hasMatch();
}

QList<ExportResourceInfo> getExportDeviceResources(const QString &syspath) {
    QList<ExportResourceInfo> resources;

    if (syspath.isEmpty() || !isPciDeviceForExport(syspath)) {
        return resources;
    }

    // Get IRQ
    QString irq = safeReadSysfsFile(syspath + QStringLiteral("/irq"));
    if (!irq.isEmpty() && irq != QStringLiteral("0")) {
        ExportResourceInfo res;
        res.type = QStringLiteral("IRQ");
        res.value = irq.toInt();
        res.displayValue =
            QStringLiteral("0x%1 (%2)").arg(res.value, 8, 16, QLatin1Char('0')).arg(irq).toUpper();
        resources.append(res);
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
                    ExportResourceInfo res;

                    // IORESOURCE_IO = 0x00000100, IORESOURCE_MEM = 0x00000200
                    if (flags & 0x00000100) {
                        res.type = QStringLiteral("I/O Range");
                    } else if (flags & 0x00000200) {
                        res.type = QStringLiteral("Memory Range");
                    } else {
                        continue;
                    }

                    res.start = QString::number(start, 16).toUpper();
                    res.end = QString::number(end, 16).toUpper();
                    res.flags = QString::number(flags, 16).toUpper();
                    res.displayValue = QStringLiteral("%1 - %2")
                                           .arg(start, 16, 16, QLatin1Char('0'))
                                           .arg(end, 16, 16, QLatin1Char('0'))
                                           .toUpper();
                    resources.append(res);
                }
            }
        }
    }

    return resources;
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
                driverInfo.filename = value;
            } else if (key == QStringLiteral("author")) {
                driverInfo.author = value;
            } else if (key == QStringLiteral("version")) {
                driverInfo.version = value;
            } else if (key == QStringLiteral("license")) {
                driverInfo.license = value;
            } else if (key == QStringLiteral("description")) {
                driverInfo.description = value;
            } else if (key == QStringLiteral("signer")) {
                driverInfo.signer = value;
            } else if (key == QStringLiteral("srcversion")) {
                driverInfo.srcversion = value;
            } else if (key == QStringLiteral("vermagic")) {
                driverInfo.vermagic = value;
            }
        }

        // Determine if out-of-tree module
        driverInfo.isOutOfTree = !driverInfo.filename.isEmpty() &&
                                 driverInfo.filename != QStringLiteral("(builtin)") &&
                                 !driverInfo.filename.contains(QStringLiteral("/kernel/"));
        driverInfo.isBuiltin = (driverInfo.filename == QStringLiteral("(builtin)"));
    }

    return driverInfo;
}

QHash<QString, QString> getSystemResourcesRaw() {
    QHash<QString, QString> resources;

    QFile dmaFile(QStringLiteral("/proc/dma"));
    if (dmaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        resources[QStringLiteral("dma")] = QString::fromLocal8Bit(dmaFile.readAll());
        dmaFile.close();
    }

    QFile ioportsFile(QStringLiteral("/proc/ioports"));
    if (ioportsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        resources[QStringLiteral("ioports")] = QString::fromLocal8Bit(ioportsFile.readAll());
        ioportsFile.close();
    }

    QFile interruptsFile(QStringLiteral("/proc/interrupts"));
    if (interruptsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        resources[QStringLiteral("interrupts")] = QString::fromLocal8Bit(interruptsFile.readAll());
        interruptsFile.close();
    }

    QFile iomemFile(QStringLiteral("/proc/iomem"));
    if (iomemFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        resources[QStringLiteral("iomem")] = QString::fromLocal8Bit(iomemFile.readAll());
        iomemFile.close();
    }

    return resources;
}

// Global udev manager for enumeration
static UdevManager &getGlobalManager() {
    static UdevManager manager;
    return manager;
}

QList<DeviceInfo> enumerateAllDevices() {
    QList<DeviceInfo> devices;
    auto &manager = getGlobalManager();

    auto *enumerator = udev_enumerate_new(manager.context());
    udev_enumerate_scan_devices(enumerator);
    struct udev_list_entry *listEntry;
    udev_list_entry_foreach(listEntry, udev_enumerate_get_list_entry(enumerator)) {
        const char *syspath = udev_list_entry_get_name(listEntry);
        auto *d = createDeviceInfo(manager.context(), syspath);
        if (d) {
            devices.emplaceBack(d);
        }
    }
    udev_enumerate_unref(enumerator);

    return devices;
}

QObject *createDeviceMonitor(QObject *parent) {
    return new UdevMonitor(getGlobalManager().context(), parent);
}
