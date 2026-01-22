#include "propertiesdialog.h"
#include "const_strings.h"
#include "devicecache.h"
#include "driverdetailsdialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QTableView>
#include <QTextStream>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace {
    // Helper to safely read a small sysfs file without blocking
    // Returns empty string on failure or if would block
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
} // namespace
#elif defined(Q_OS_WIN)
#include <windows.h>
#include <versionhelpers.h>
#endif

namespace props = strings::udev::propertyNames;

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent), deviceInfo_(nullptr), eventsModel_(nullptr) {
    setupUi(this);
    setMinimumSize(493, 502);

    // Make General tab value labels selectable
    labelDeviceTypeValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelManufacturerValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelLocationValue->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Make device name labels selectable on all tabs
    labelDeviceNameGeneral->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelDeviceNameDriver->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelDeviceNameDetails->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelDeviceNameEvents->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Setup property mappings for Details tab
    propertyMappings_ = {
        {tr("Device description"), QStringLiteral("DEVICE_DESCRIPTION"), false},
        {tr("Hardware IDs"), QStringLiteral("MODALIAS"), false},
        {tr("Compatible IDs"), QStringLiteral("COMPATIBLE_IDS"), true},
        {tr("Device class"), QStringLiteral("ID_PCI_CLASS_FROM_DATABASE"), false},
        {tr("Device class GUID"), QStringLiteral("ID_PCI_CLASS"), false},
        {tr("Driver key"), QStringLiteral("DRIVER"), false},
        {tr("Device instance path"), QStringLiteral("DEVPATH"), false},
        {tr("Location paths"), QStringLiteral("DEVPATH"), false},
        {tr("Physical device object name"), QStringLiteral("DEVNAME"), false},
        {tr("Bus relations"), QStringLiteral("SUBSYSTEM"), false},
        {tr("Parent"), QStringLiteral("PARENT_SYSPATH"), false},
        {tr("Children"), QStringLiteral("CHILDREN"), true},
        {tr("Manufacturer"), QStringLiteral("ID_VENDOR_FROM_DATABASE"), false},
        {tr("Model"), QStringLiteral("ID_MODEL_FROM_DATABASE"), false},
        {tr("Serial number"), QStringLiteral("ID_SERIAL"), false},
        {tr("Subsystem"), QStringLiteral("SUBSYSTEM"), false},
        {tr("Device node"), QStringLiteral("DEVNAME"), false},
        {tr("Syspath"), QStringLiteral("SYSPATH"), false},
        {tr("Mount point"), QStringLiteral("MOUNT_POINT"), false},
    };

    // Connect property dropdown
    connect(comboBoxDetailsProperty,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &PropertiesDialog::onPropertySelectionChanged);

    // Setup events table model
    eventsModel_ = new QStandardItemModel(this);
    eventsModel_->setHorizontalHeaderLabels({tr("Timestamp"), tr("Event")});
    tableViewEvents->setModel(eventsModel_);
    tableViewEvents->horizontalHeader()->setStretchLastSection(true);
    tableViewEvents->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableViewEvents->setSelectionMode(QAbstractItemView::SingleSelection);

    // Connect event selection
    connect(tableViewEvents->selectionModel(),
            &QItemSelectionModel::currentRowChanged,
            this,
            &PropertiesDialog::onEventSelectionChanged);

    // Connect driver details button
    connect(buttonDriverDetails,
            &QPushButton::clicked,
            this,
            &PropertiesDialog::onDriverDetailsClicked);

    // Connect view all events button
    connect(buttonViewAllEvents,
            &QPushButton::clicked,
            this,
            &PropertiesDialog::onViewAllEventsClicked);

    // Shortcut: Ctrl+P copies device path when on General tab
    auto *copyPathShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P), this);
    copyPathShortcut->setContext(Qt::WindowShortcut);
    connect(copyPathShortcut, &QShortcut::activated, this, &PropertiesDialog::onCopyDevicePath);
}

PropertiesDialog::~PropertiesDialog() {
    // Cancel any pending async operations
    if (eventsWatcher_) {
        eventsWatcher_->disconnect();
        eventsWatcher_->cancel();
        eventsWatcher_->waitForFinished();
        delete eventsWatcher_;
        eventsWatcher_ = nullptr;
    }
}

void PropertiesDialog::setDeviceSyspath(const QString &syspath) {
    syspath_ = syspath;
    deviceInfo_ = DeviceCache::instance().deviceBySyspath(syspath);

    if (!deviceInfo_) {
        return;
    }

    // Check if this is the Computer (DMI) entry
    bool isComputerEntry = (syspath == QStringLiteral("/sys/devices/virtual/dmi/id"));

    QString deviceName;
    if (isComputerEntry) {
        // Determine if ACPI or device-tree based
        QFileInfo acpiInfo(QStringLiteral("/sys/firmware/acpi"));
        QFileInfo dtInfo(QStringLiteral("/sys/firmware/devicetree"));

        if (acpiInfo.exists() && acpiInfo.isDir()) {
#if defined(Q_PROCESSOR_X86_64)
            deviceName = tr("ACPI x64-based PC");
#elif defined(Q_PROCESSOR_X86_32)
            deviceName = tr("ACPI x86-based PC");
#elif defined(Q_PROCESSOR_ARM_64)
            deviceName = tr("ACPI ARM64-based PC");
#elif defined(Q_PROCESSOR_ARM)
            deviceName = tr("ACPI ARM-based PC");
#else
            deviceName = tr("ACPI-based PC");
#endif
        } else if (dtInfo.exists() && dtInfo.isDir()) {
            deviceName = tr("Device Tree-based System");
        } else {
            deviceName = tr("Standard PC");
        }
    } else {
        deviceName = deviceInfo_->name();
        // Apply nice name mapping for software and input devices
        if (deviceInfo_->subsystem() == strings::udev::subsystems::misc() ||
            deviceInfo_->subsystem() == strings::udev::subsystems::input()) {
            deviceName = strings::softwareDeviceNiceName(deviceName);
        }
        // For storage volumes, try partition label or filesystem label
        if (deviceInfo_->subsystem() == strings::udev::subsystems::block() &&
            deviceInfo_->propertyValue(strings::udev::propertyNames::DEVTYPE) ==
                strings::udev::propertyValues::devType::partition()) {
            QString partName =
                deviceInfo_->propertyValue(strings::udev::propertyNames::ID_PART_ENTRY_NAME);
            if (partName.isEmpty()) {
                partName = deviceInfo_->propertyValue(strings::udev::propertyNames::ID_FS_LABEL);
            }
            if (!partName.isEmpty()) {
                deviceName = partName;
            }
        }
    }

    // Set device name labels on all tabs
    labelDeviceNameGeneral->setText(deviceName);
    labelDeviceNameDetails->setText(deviceName);
    labelDeviceNameDriver->setText(deviceName);
    labelDeviceNameEvents->setText(deviceName);

    // Set window title
    setWindowTitle(tr("%1 Properties").arg(deviceName));

    if (isComputerEntry) {
        // Special handling for Computer entry
        labelDeviceTypeValue->setText(tr("Computer"));
        labelManufacturerValue->setText(tr("(Standard computers)"));
        labelLocationValue->setText(tr("Unknown"));
        labelLocationValue->setToolTip(QString());

        // Hide Events and Details tabs
        int detailsIndex = tabWidget->indexOf(tabDetails);
        if (detailsIndex >= 0) {
            tabWidget->removeTab(detailsIndex);
        }
        int eventsIndex = tabWidget->indexOf(tab); // Events tab is named "tab" in UI
        if (eventsIndex >= 0) {
            tabWidget->removeTab(eventsIndex);
        }

        // Populate only General and Driver tabs
        populateDriverTab();
    } else {
        // Populate all tabs
        populateGeneralTab();
        populateDriverTab();
        populateDetailsTab();
        populateEventsTab();

        // Create Resources tab if device has resources
        createResourcesTab();
    }
}

void PropertiesDialog::setCategoryIcon(const QIcon &icon) {
    categoryIcon_ = icon;
    QPixmap pixmap = icon.pixmap(32, 32);

    // Set icon on all tabs
    labelIconGeneral->setPixmap(pixmap);
    labelIconDriver->setPixmap(pixmap);
    labelIconDetails->setPixmap(pixmap);
    labelIconEvents->setPixmap(pixmap);
}

void PropertiesDialog::populateGeneralTab() {
    if (!deviceInfo_)
        return;

    // Device type - use category name matching "Devices by type" view
    QString deviceType = getDeviceCategory();
    if (!deviceType.isEmpty()) {
        labelDeviceTypeValue->setText(deviceType);
    }

    // Manufacturer - try multiple sources
    QString manufacturer;

    // Storage volumes (partitions) don't have a manufacturer
    bool isStorageVolume = (deviceInfo_->subsystem() == strings::udev::subsystems::block() &&
                            deviceInfo_->propertyValue(props::DEVTYPE) ==
                                strings::udev::propertyValues::devType::partition());

    if (isStorageVolume) {
        manufacturer = tr("N/A");
    } else {
        manufacturer = deviceInfo_->propertyValue(props::ID_VENDOR_FROM_DATABASE);
    }

    // For block devices (disk drives, DVD/CD-ROM), try harder to get manufacturer
    if (manufacturer.isEmpty() && deviceInfo_->subsystem() == strings::udev::subsystems::block()) {
        manufacturer = getBlockDeviceManufacturer();
    }
    if (manufacturer.isEmpty()) {
        // Try USB-specific vendor properties
        manufacturer = deviceInfo_->propertyValue(props::ID_USB_VENDOR);
    }
    if (manufacturer.isEmpty()) {
        manufacturer = deviceInfo_->propertyValue(props::ID_VENDOR);
    }
    if (manufacturer.isEmpty()) {
        // Try URL-encoded vendor and decode it
        QString encoded = deviceInfo_->propertyValue(props::ID_VENDOR_ENC);
        if (!encoded.isEmpty()) {
            manufacturer = QUrl::fromPercentEncoding(encoded.toUtf8());
            // Replace underscores with spaces (udev convention)
            manufacturer.replace(QLatin1Char('_'), QLatin1Char(' '));
        }
    }
    if (manufacturer.isEmpty()) {
        // Try to parse vendor ID from device paths
        QString syspath = deviceInfo_->syspath();
        QString vendorId;

        // Try uhid path: /devices/virtual/misc/uhid/xxxx:yyyy:zzzz
        // Format is typically BUS:VENDOR:PRODUCT (e.g., 0003:054C:0CE6)
        static const QRegularExpression uhidRe(QStringLiteral(
            "/devices/virtual/misc/uhid/[0-9a-fA-F]{4}:([0-9a-fA-F]{4}):[0-9a-fA-F]{4}"));
        auto match = uhidRe.match(syspath);
        if (match.hasMatch()) {
            vendorId = match.captured(1).toLower();
        }

        // Try USB device path: /devices/pci.../usbN/N-N/.../BUS:VENDOR_ID:DEV_ID.XXXX
        // e.g., /devices/pci0000:00/0000:00:14.0/usb1/1-4/1-4:1.2/0003:046D:C52B.0001
        if (vendorId.isEmpty()) {
            static const QRegularExpression usbDevRe(
                QStringLiteral("[0-9a-fA-F]{4}:([0-9a-fA-F]{4}):[0-9a-fA-F]{4}\\.[0-9a-fA-F]{4}$"));
            match = usbDevRe.match(syspath);
            if (match.hasMatch()) {
                vendorId = match.captured(1).toLower();
            }
        }

        if (!vendorId.isEmpty()) {
            manufacturer = lookupUsbVendor(vendorId);
        }
    }
    // Fallback for known vendors not in database or showing raw ID
    if (manufacturer == QStringLiteral("046d")) {
        manufacturer = QStringLiteral("Logitech, Inc.");
    }
    // Clean up ugly manufacturer names
    if (manufacturer == QStringLiteral("Metadot_-_Das_Keyboard") ||
        manufacturer == QStringLiteral("Metadot - Das Keyboard")) {
        manufacturer = QStringLiteral("Metadot");
    }

    // Override manufacturer based on device name for known devices
    QString devName = deviceInfo_->name();
    QString devNode = deviceInfo_->devnode();
    // Strip /dev/ prefix for comparison
    QString shortName = devName;
    if (shortName.startsWith(QStringLiteral("/dev/"))) {
        shortName = shortName.mid(5);
    }
    QString shortNode = devNode;
    if (shortNode.startsWith(QStringLiteral("/dev/"))) {
        shortNode = shortNode.mid(5);
    }

    // Oracle/VirtualBox devices (only vbox* devices)
    if (shortName.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive) ||
        shortNode.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive)) {
        manufacturer = QStringLiteral("Oracle Corporation");
    }
    // Linux Foundation devices (misc/software devices and input devices)
    else if (shortName == QStringLiteral("autofs") ||
             shortName == QStringLiteral("cpu dma latency") ||
             shortName == QStringLiteral("cpu_dma_latency") ||
             shortName == QStringLiteral("fuse") || shortName == QStringLiteral("hpet") ||
             shortName == QStringLiteral("hwrng") || shortName == QStringLiteral("kvm") ||
             shortName == QStringLiteral("loop-control") ||
             shortName == QStringLiteral("loop control") || shortName == QStringLiteral("mcelog") ||
             shortName == QStringLiteral("net/tun") || shortName == QStringLiteral("ntsync") ||
             shortName == QStringLiteral("rfkill") || shortName == QStringLiteral("uhid") ||
             shortName == QStringLiteral("uinput") || shortName == QStringLiteral("vga arbiter") ||
             shortName == QStringLiteral("vhost-net") ||
             shortName.startsWith(QStringLiteral("input/event")) ||
             shortName.startsWith(QStringLiteral("input/mouse")) ||
             shortNode == QStringLiteral("autofs") ||
             shortNode == QStringLiteral("cpu_dma_latency") ||
             shortNode == QStringLiteral("fuse") || shortNode == QStringLiteral("hpet") ||
             shortNode == QStringLiteral("hwrng") || shortNode == QStringLiteral("kvm") ||
             shortNode == QStringLiteral("loop-control") || shortNode == QStringLiteral("mcelog") ||
             shortNode == QStringLiteral("net/tun") || shortNode == QStringLiteral("ntsync") ||
             shortNode == QStringLiteral("rfkill") || shortNode == QStringLiteral("uhid") ||
             shortNode == QStringLiteral("uinput") ||
             shortNode.startsWith(QStringLiteral("input/event")) ||
             shortNode.startsWith(QStringLiteral("input/mouse")) ||
             shortNode == QStringLiteral("vga_arbiter") ||
             shortNode == QStringLiteral("vhost-net") ||
             shortName == QStringLiteral("mapper/control") ||
             shortNode == QStringLiteral("mapper/control")) {
        manufacturer = QStringLiteral("Linux Foundation");
    }

    if (!manufacturer.isEmpty()) {
        labelManufacturerValue->setText(manufacturer);
    }

    // Location - translate devpath to human-readable form
    QString devpath = deviceInfo_->propertyValue(props::DEVPATH);
    QString location = translateLocation(devpath);
    if (!location.isEmpty()) {
        labelLocationValue->setText(location);
        labelLocationValue->setToolTip(devpath);

        // Override manufacturer for virtual devices if it would be Unknown
        if (location == tr("Virtual device") &&
            labelManufacturerValue->text() == QStringLiteral("Unknown")) {
            labelManufacturerValue->setText(QStringLiteral("Linux Foundation"));
        }
    }

    // Device status - if the device exists and has a driver, it's working
    QString driver = deviceInfo_->driver();
    if (!driver.isEmpty()) {
        textEditDeviceStatus->setPlainText(tr("This device is working properly."));
    } else {
        textEditDeviceStatus->setPlainText(tr("No driver installed for this device."));
    }
}

void PropertiesDialog::populateDriverTab() {
    if (!deviceInfo_)
        return;

    QString driver = deviceInfo_->driver();

    // Check if this is a Linux Foundation device (software/misc devices)
    bool isLinuxFoundationDevice = false;
    QString devName = deviceInfo_->name();
    QString devNode = deviceInfo_->devnode();
    QString shortName = devName.startsWith(QStringLiteral("/dev/")) ? devName.mid(5) : devName;
    QString shortNode = devNode.startsWith(QStringLiteral("/dev/")) ? devNode.mid(5) : devNode;

    if (shortName.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive) ||
        shortNode.startsWith(QStringLiteral("vbox"), Qt::CaseInsensitive)) {
        // VirtualBox devices are not Linux Foundation
        isLinuxFoundationDevice = false;
    } else if (deviceInfo_->subsystem() == strings::udev::subsystems::misc() ||
               deviceInfo_->subsystem() == strings::udev::subsystems::input()) {
        // Most misc/software devices and input devices are Linux Foundation
        isLinuxFoundationDevice = true;
    } else if (deviceInfo_->subsystem() == strings::udev::subsystems::block() &&
               deviceInfo_->propertyValue(strings::udev::propertyNames::DEVTYPE) ==
                   strings::udev::propertyValues::devType::partition()) {
        // Storage volumes (partitions) are Linux Foundation
        isLinuxFoundationDevice = true;
    }

    // For Linux, get driver info from modinfo for out-of-tree modules
    bool isKernelDriver = !driver.isEmpty() || isLinuxFoundationDevice;

    if (isKernelDriver) {
#ifdef Q_OS_LINUX
        QString driverProvider = QStringLiteral("Linux Foundation");
        QString driverVersion = getKernelVersion();
        QString driverSigner = QStringLiteral("Linux Foundation");
        bool hasDriverFiles = false;

        if (!driver.isEmpty()) {
            // Get module info to check if it's an out-of-tree module
            QProcess modinfo;
            modinfo.start(QStringLiteral("modinfo"), {driver});
            if (modinfo.waitForFinished(3000)) {
                QString output = QString::fromUtf8(modinfo.readAllStandardOutput());
                QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

                QString filename;
                QString author;
                QString version;
                QString license;
                QString signer;

                for (const QString &line : lines) {
                    int colonIdx = line.indexOf(QLatin1Char(':'));
                    if (colonIdx < 0)
                        continue;

                    QString key = line.left(colonIdx).trimmed();
                    QString value = line.mid(colonIdx + 1).trimmed();

                    if (key == QStringLiteral("filename")) {
                        filename = value;
                    } else if (key == QStringLiteral("author") && author.isEmpty()) {
                        author = value;
                    } else if (key == QStringLiteral("version")) {
                        version = value;
                    } else if (key == QStringLiteral("license")) {
                        license = value;
                    } else if (key == QStringLiteral("signer")) {
                        signer = value;
                    }
                }

                hasDriverFiles = !filename.isEmpty();

                // Check if this is an out-of-tree module (not in /lib/modules/.../kernel/)
                bool isOutOfTree = !filename.isEmpty() && filename != QStringLiteral("(builtin)") &&
                                   !filename.contains(QStringLiteral("/kernel/"));

                // Special handling for nvidia driver
                if (driver == QStringLiteral("nvidia") ||
                    driver.startsWith(QStringLiteral("nvidia_"))) {
                    driverProvider = QStringLiteral("NVIDIA Corporation");
                    if (!version.isEmpty()) {
                        driverVersion = version;
                    }
                    driverSigner = QStringLiteral("NVIDIA Corporation");
                } else if (isOutOfTree) {
                    // For other out-of-tree modules, use author if available
                    if (!author.isEmpty()) {
                        driverProvider = author;
                    }
                    if (!version.isEmpty()) {
                        driverVersion = version;
                    }
                    if (!signer.isEmpty()) {
                        driverSigner = signer;
                    } else if (!author.isEmpty()) {
                        driverSigner = author;
                    }
                } else {
                    // In-tree module - use version from modinfo if available
                    if (!version.isEmpty()) {
                        driverVersion = version;
                    }
                    if (!signer.isEmpty()) {
                        driverSigner = signer;
                    }
                }
            }
        }

        labelDriverProviderValue->setText(driverProvider);
        labelDriverVersionValue->setText(driverVersion);
        labelDigitalSignerValue->setText(driverSigner);
        labelDriverDateValue->setText(getKernelBuildDate());
        buttonDriverDetails->setEnabled(hasDriverFiles);
#elif defined(Q_OS_MACOS)
        // macOS: Get driver info from kext
        QString driverProvider = QStringLiteral("Apple Inc.");
        QString driverVersion = getKernelVersion();
        QString driverSigner = QStringLiteral("Apple Inc.");
        bool hasDriverDetails = false;

        if (!driver.isEmpty()) {
            // Check if this is a kext bundle identifier
            if (driver.contains(QLatin1Char('.'))) {
                // Try to get kext info using kextstat
                QProcess kextstat;
                kextstat.start(QStringLiteral("kextstat"), {QStringLiteral("-b"), driver});
                if (kextstat.waitForFinished(3000)) {
                    QString output = QString::fromUtf8(kextstat.readAllStandardOutput());
                    if (!output.isEmpty() && output.contains(driver)) {
                        hasDriverDetails = true;

                        // Parse version from kextstat output (format: Index Refs Address Size Wired Name (Version))
                        QRegularExpression versionRe(QStringLiteral("\\(([^)]+)\\)$"));
                        QRegularExpressionMatch match = versionRe.match(output.trimmed());
                        if (match.hasMatch()) {
                            driverVersion = match.captured(1);
                        }

                        // Determine provider based on bundle identifier
                        if (driver.startsWith(QStringLiteral("com.apple."))) {
                            driverProvider = QStringLiteral("Apple Inc.");
                            driverSigner = QStringLiteral("Apple Inc.");
                        } else {
                            // Third-party kext - extract vendor from bundle ID
                            QStringList parts = driver.split(QLatin1Char('.'));
                            if (parts.size() >= 2) {
                                driverProvider = parts.at(1);
                                driverProvider[0] = driverProvider[0].toUpper();
                            }
                            driverSigner = driverProvider;
                        }
                    }
                }
            } else {
                // Just an IOKit class name, use as-is
                driverProvider = QStringLiteral("Apple Inc.");
                hasDriverDetails = false;
            }
        }

        labelDriverProviderValue->setText(driverProvider);
        labelDriverVersionValue->setText(driverVersion);
        labelDigitalSignerValue->setText(driverSigner);
        labelDriverDateValue->setText(getKernelBuildDate());
        buttonDriverDetails->setEnabled(hasDriverDetails);
#elif defined(Q_OS_WIN)
        // Windows: Get driver info from the registry
        QString driverProvider = QStringLiteral("Microsoft");
        QString driverVersion = getKernelVersion();
        QString driverSigner = QStringLiteral("Microsoft Windows");
        bool hasDriverDetails = false;

        // The driver key points to the registry location
        // Format: {GUID}\NNNN
        if (!driver.isEmpty()) {
            QString driverKeyPath = QStringLiteral("SYSTEM\\CurrentControlSet\\Control\\Class\\") + driver;
            HKEY hKey;
            std::wstring keyPath = driverKeyPath.toStdWString();

            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                wchar_t value[256];
                DWORD valueSize = sizeof(value);
                DWORD type;

                // Get provider name
                if (RegQueryValueExW(hKey, L"ProviderName", nullptr, &type,
                                     reinterpret_cast<LPBYTE>(value), &valueSize) == ERROR_SUCCESS) {
                    if (type == REG_SZ) {
                        driverProvider = QString::fromWCharArray(value);
                        hasDriverDetails = true;
                    }
                }

                // Get driver version
                valueSize = sizeof(value);
                if (RegQueryValueExW(hKey, L"DriverVersion", nullptr, &type,
                                     reinterpret_cast<LPBYTE>(value), &valueSize) == ERROR_SUCCESS) {
                    if (type == REG_SZ) {
                        driverVersion = QString::fromWCharArray(value);
                    }
                }

                // Get driver date
                valueSize = sizeof(value);
                if (RegQueryValueExW(hKey, L"DriverDate", nullptr, &type,
                                     reinterpret_cast<LPBYTE>(value), &valueSize) == ERROR_SUCCESS) {
                    if (type == REG_SZ) {
                        labelDriverDateValue->setText(QString::fromWCharArray(value));
                    }
                }

                // Get digital signer (InfPath can give clues about signing)
                valueSize = sizeof(value);
                if (RegQueryValueExW(hKey, L"InfPath", nullptr, &type,
                                     reinterpret_cast<LPBYTE>(value), &valueSize) == ERROR_SUCCESS) {
                    if (type == REG_SZ) {
                        QString infPath = QString::fromWCharArray(value);
                        // If it's in the Windows\INF folder, it's likely Microsoft signed
                        if (infPath.startsWith(QStringLiteral("oem"), Qt::CaseInsensitive)) {
                            driverSigner = driverProvider;
                        } else {
                            driverSigner = QStringLiteral("Microsoft Windows");
                        }
                    }
                }

                RegCloseKey(hKey);
            }
        }

        labelDriverProviderValue->setText(driverProvider);
        labelDriverVersionValue->setText(driverVersion);
        labelDigitalSignerValue->setText(driverSigner);
        buttonDriverDetails->setEnabled(hasDriverDetails);
#else
        labelDriverProviderValue->setText(driver);
        labelDigitalSignerValue->setText(tr("N/A"));
        labelDriverVersionValue->setText(tr("N/A"));
        labelDriverDateValue->setText(tr("N/A"));
        buttonDriverDetails->setEnabled(false);
#endif
    } else {
        labelDriverProviderValue->setText(tr("(No driver)"));
        labelDigitalSignerValue->setText(tr("N/A"));
        labelDriverVersionValue->setText(tr("N/A"));
        labelDriverDateValue->setText(tr("N/A"));
        buttonDriverDetails->setEnabled(false);
    }
}

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
QString PropertiesDialog::getKernelVersion() {
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        return QString::fromLocal8Bit(buffer.release);
    }
    return tr("Unknown");
}

QString PropertiesDialog::getKernelBuildDate() {
#ifdef Q_OS_LINUX
    // Try to get kernel build date from /proc/version
    QFile versionFile(QStringLiteral("/proc/version"));
    if (versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(versionFile.readAll());
        versionFile.close();

        // Parse build date from /proc/version
        // Format typically: "Linux version X.X.X (user@host) (gcc ...) #N SMP PREEMPT_DYNAMIC Wed Jan 15 17:42:44 UTC 2025"
        // Try to match: "Day Mon DD HH:MM:SS TZ YYYY" or "Day Mon DD HH:MM:SS YYYY"
        QRegularExpression dateRe(
            QStringLiteral("(Mon|Tue|Wed|Thu|Fri|Sat|Sun)\\s+"
                           "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\s+"
                           "(\\d{1,2})\\s+"
                           "(\\d{2}):(\\d{2}):(\\d{2})\\s+"
                           "(?:\\w+\\s+)?" // Optional timezone
                           "(\\d{4})"));

        QRegularExpressionMatch match = dateRe.match(content);
        if (match.hasMatch()) {
            QString monthStr = match.captured(2);
            int day = match.captured(3).toInt();
            int year =
                match.captured(7).toInt(); // Group 7: year (after hour/min/sec in groups 4/5/6)

            // Convert month string to number
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
            int month = months.indexOf(monthStr) + 1;

            if (month > 0) {
                QDate date(year, month, day);
                if (date.isValid()) {
                    // Format according to user's locale
                    return QLocale().toString(date, QLocale::ShortFormat);
                }
            }
        }

        // Fallback: just return the version string portion
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            return QString::fromLocal8Bit(buffer.version);
        }
    }
#elif defined(Q_OS_MACOS)
    // macOS: Parse date from uname -v (version string contains build date)
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        QString version = QString::fromLocal8Bit(buffer.version);
        // Format: "Darwin Kernel Version X.X.X: Day Mon DD HH:MM:SS TZ YYYY; root:xnu-..."
        QRegularExpression dateRe(
            QStringLiteral("(Mon|Tue|Wed|Thu|Fri|Sat|Sun)\\s+"
                           "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec)\\s+"
                           "(\\d{1,2})\\s+"
                           "(\\d{2}):(\\d{2}):(\\d{2})\\s+"
                           "\\w+\\s+" // Timezone
                           "(\\d{4})"));

        QRegularExpressionMatch match = dateRe.match(version);
        if (match.hasMatch()) {
            QString monthStr = match.captured(2);
            int day = match.captured(3).toInt();
            int year = match.captured(7).toInt();

            static const QStringList months = {
                QStringLiteral("Jan"), QStringLiteral("Feb"), QStringLiteral("Mar"),
                QStringLiteral("Apr"), QStringLiteral("May"), QStringLiteral("Jun"),
                QStringLiteral("Jul"), QStringLiteral("Aug"), QStringLiteral("Sep"),
                QStringLiteral("Oct"), QStringLiteral("Nov"), QStringLiteral("Dec")};
            int month = months.indexOf(monthStr) + 1;

            if (month > 0) {
                QDate date(year, month, day);
                if (date.isValid()) {
                    return QLocale().toString(date, QLocale::ShortFormat);
                }
            }
        }
        return version;
    }
#endif
    return tr("Unknown");
}
#elif defined(Q_OS_WIN)
QString PropertiesDialog::getKernelVersion() {
    // Get Windows version using RtlGetVersion (works on Windows 8.1+)
    // Fallback to reading from registry if needed
    OSVERSIONINFOEXW osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    // Try to get version via ntdll
    typedef NTSTATUS(WINAPI * RtlGetVersionFunc)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll) {
        auto rtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(
            GetProcAddress(ntdll, "RtlGetVersion"));
        if (rtlGetVersion) {
            rtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi));
            return QStringLiteral("%1.%2.%3")
                .arg(osvi.dwMajorVersion)
                .arg(osvi.dwMinorVersion)
                .arg(osvi.dwBuildNumber);
        }
    }

    return tr("Unknown");
}

QString PropertiesDialog::getKernelBuildDate() {
    // Windows doesn't have a simple kernel build date
    // Return the OS install date or a generic message
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD installDate = 0;
        DWORD size = sizeof(installDate);
        if (RegQueryValueExW(hKey, L"InstallDate", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(&installDate), &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            // InstallDate is a Unix timestamp
            QDateTime dt = QDateTime::fromSecsSinceEpoch(installDate);
            return QLocale().toString(dt.date(), QLocale::ShortFormat);
        }
        RegCloseKey(hKey);
    }
    return tr("N/A");
}
#else
QString PropertiesDialog::getKernelVersion() {
    return tr("N/A");
}

QString PropertiesDialog::getKernelBuildDate() {
    return tr("N/A");
}
#endif

QString PropertiesDialog::lookupUsbVendor(const QString &vendorId) {
    // Cache the USB IDs database on first use
    static QHash<QString, QString> vendorCache;
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        static const QStringList usbIdsLocations = {
            // Linux locations
            QStringLiteral("/usr/share/hwdata/usb.ids"),
            QStringLiteral("/usr/share/misc/usb.ids"),
            QStringLiteral("/usr/share/usb.ids"),
            QStringLiteral("/var/lib/usbutils/usb.ids"),
#ifdef Q_OS_MACOS
            // macOS Homebrew locations (Apple Silicon and Intel)
            QStringLiteral("/opt/homebrew/share/hwdata/usb.ids"),
            QStringLiteral("/usr/local/share/hwdata/usb.ids"),
            QStringLiteral("/opt/homebrew/Cellar/hwdata/*/share/hwdata/usb.ids"),
#endif
        };

        for (const QString &path : usbIdsLocations) {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }

            QTextStream in(&file);
            // USB IDs format: vendor ID at start of line (4 hex chars), then two spaces, then name
            // e.g., "054c  Sony Corp."
            while (!in.atEnd()) {
                QString line = in.readLine();
                // Vendor lines start with 4 hex digits (no leading whitespace)
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

            // Stop after first successful file
            if (!vendorCache.isEmpty()) {
                break;
            }
        }
    }

    return vendorCache.value(vendorId.toLower());
}

QString PropertiesDialog::translateLocation(const QString &devpath) {
    if (devpath.isEmpty()) {
        return tr("Unknown");
    }

#ifdef Q_OS_MACOS
    // macOS: Parse IORegistry paths
    // Format: IOService:/AppleARMPE/arm-io@10F00000/AppleT810xIO/usb-drd1@2280000/...

    // Check for USB device
    if (devpath.contains(QStringLiteral("USB")) || devpath.contains(QStringLiteral("usb"))) {
        // Try to extract USB location from path
        static const QRegularExpression usbLocationRe(
            QStringLiteral("usb[^/]*/([^@/]+)@([0-9a-fA-F]+)"));
        auto match = usbLocationRe.match(devpath);
        if (match.hasMatch()) {
            return tr("On USB bus");
        }
        return tr("On USB bus");
    }

    // Check for PCI device
    if (devpath.contains(QStringLiteral("PCI")) || devpath.contains(QStringLiteral("pci"))) {
        return tr("On PCI bus");
    }

    // Check for Thunderbolt
    if (devpath.contains(QStringLiteral("Thunderbolt"))) {
        return tr("On Thunderbolt bus");
    }

    // Check for built-in devices
    if (devpath.contains(QStringLiteral("AppleARMPE")) ||
        devpath.contains(QStringLiteral("arm-io"))) {
        return tr("On system board");
    }

    // Check for AppleInternal / built-in
    if (devpath.contains(QStringLiteral("Apple"))) {
        return tr("Built-in");
    }

    return tr("Unknown");

#elif defined(Q_OS_WIN)
    // Windows: Parse device instance ID paths
    // Format: BUS\VEN_XXXX&DEV_YYYY&SUBSYS_ZZZZ&REV_WW\Instance

    // Check for PCI device
    if (devpath.startsWith(QStringLiteral("PCI\\"), Qt::CaseInsensitive)) {
        // Extract PCI location from instance ID if available
        // Format: PCI\VEN_8086&DEV_A348&SUBSYS_...
        QRegularExpression pciVenDevRe(QStringLiteral("VEN_([0-9A-Fa-f]{4})&DEV_([0-9A-Fa-f]{4})"));
        auto match = pciVenDevRe.match(devpath);
        if (match.hasMatch()) {
            return tr("On PCI bus");
        }
        return tr("PCI device");
    }

    // Check for USB device
    if (devpath.startsWith(QStringLiteral("USB\\"), Qt::CaseInsensitive)) {
        // Extract USB location
        // Format: USB\VID_XXXX&PID_YYYY\Serial
        QRegularExpression usbVidPidRe(QStringLiteral("VID_([0-9A-Fa-f]{4})&PID_([0-9A-Fa-f]{4})"));
        auto match = usbVidPidRe.match(devpath);
        if (match.hasMatch()) {
            return tr("On USB bus");
        }
        return tr("USB device");
    }

    // Check for HID device
    if (devpath.startsWith(QStringLiteral("HID\\"), Qt::CaseInsensitive)) {
        return tr("HID-compliant device");
    }

    // Check for ACPI device
    if (devpath.startsWith(QStringLiteral("ACPI\\"), Qt::CaseInsensitive) ||
        devpath.startsWith(QStringLiteral("ACPI_HAL\\"), Qt::CaseInsensitive)) {
        return tr("On ACPI-compliant system");
    }

    // Check for root-enumerated device
    if (devpath.startsWith(QStringLiteral("ROOT\\"), Qt::CaseInsensitive)) {
        return tr("On system board");
    }

    // Check for storage device
    if (devpath.contains(QStringLiteral("SCSI\\"), Qt::CaseInsensitive) ||
        devpath.contains(QStringLiteral("IDE\\"), Qt::CaseInsensitive) ||
        devpath.contains(QStringLiteral("STORAGE\\"), Qt::CaseInsensitive)) {
        return tr("On storage controller");
    }

    // Check for display device
    if (devpath.startsWith(QStringLiteral("DISPLAY\\"), Qt::CaseInsensitive)) {
        return tr("On display adapter");
    }

    // Check for software device
    if (devpath.startsWith(QStringLiteral("SWD\\"), Qt::CaseInsensitive) ||
        devpath.startsWith(QStringLiteral("SW\\"), Qt::CaseInsensitive)) {
        return tr("Software device");
    }

    return tr("Unknown");

#else // Linux
    // Check for PCI device: extract bus, device, function from patterns like 0000:00:1f.3
    static const QRegularExpression pciRe(
        QStringLiteral("([0-9a-fA-F]{4}):([0-9a-fA-F]{2}):([0-9a-fA-F]{2})\\.([0-9a-fA-F])"));

    // Find all PCI addresses in the path
    QRegularExpressionMatchIterator it = pciRe.globalMatch(devpath);
    QString lastPciMatch;
    int bus = -1, device = -1, function = -1;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        bool ok;
        bus = match.captured(2).toInt(&ok, 16);
        device = match.captured(3).toInt(&ok, 16);
        function = match.captured(4).toInt(&ok, 16);
        lastPciMatch = match.captured(0);
    }

    // Check for USB device
    // Format: /usbN/N-port or /usbN/N-port.subport.subport
    static const QRegularExpression usbRe(QStringLiteral("/usb(\\d+)/(\\d+)-([\\d.]+)"));
    auto usbMatch = usbRe.match(devpath);
    if (usbMatch.hasMatch()) {
        QString usbBus = usbMatch.captured(1);
        QString usbPort = usbMatch.captured(3); // Just the port part after "bus-"
        return tr("On USB bus %1, port %2").arg(usbBus, usbPort);
    }

    // Check for SCSI device: host, channel, target, lun
    static const QRegularExpression scsiRe(
        QStringLiteral("/host(\\d+)/target\\d+:(\\d+):(\\d+)/(\\d+):(\\d+):(\\d+):(\\d+)"));
    auto scsiMatch = scsiRe.match(devpath);
    if (scsiMatch.hasMatch()) {
        QString busNum = scsiMatch.captured(5);
        QString targetId = scsiMatch.captured(6);
        QString lun = scsiMatch.captured(7);
        return tr("Bus number %1, target ID %2, LUN %3").arg(busNum, targetId, lun);
    }

    // Check for ACPI device
    if (devpath.contains(QStringLiteral("/ACPI")) || devpath.contains(QStringLiteral("/acpi"))) {
        return tr("On ACPI-compliant system");
    }

    // Check for platform device
    if (devpath.contains(QStringLiteral("/platform/"))) {
        return tr("On system board");
    }

    // Check for virtual device
    if (devpath.contains(QStringLiteral("/virtual/"))) {
        return tr("Virtual device");
    }

    // If we found a PCI address, format it
    if (bus >= 0 && device >= 0 && function >= 0) {
        return tr("PCI bus %1, device %2, function %3").arg(bus).arg(device).arg(function);
    }

    // Check for input devices
    if (devpath.contains(QStringLiteral("/input/"))) {
        if (devpath.contains(QStringLiteral("/serio"))) {
            return tr("Plugged into keyboard port");
        }
        return tr("On input device");
    }

    // Fallback
    return tr("Unknown");
#endif
}

QString PropertiesDialog::getDeviceCategory() {
    if (!deviceInfo_) {
        return {};
    }

#ifdef Q_OS_MACOS
    // macOS: Use pre-computed category from DeviceInfo
    switch (deviceInfo_->category()) {
    case DeviceCategory::AudioInputsAndOutputs:
        return tr("Audio inputs and outputs");
    case DeviceCategory::Batteries:
        return tr("Batteries");
    case DeviceCategory::Computer:
        return tr("Computer");
    case DeviceCategory::DiskDrives:
        return tr("Disk drives");
    case DeviceCategory::DisplayAdapters:
        return tr("Display adapters");
    case DeviceCategory::DvdCdromDrives:
        return tr("DVD/CD-ROM drives");
    case DeviceCategory::HumanInterfaceDevices:
        return tr("Human Interface Devices");
    case DeviceCategory::Keyboards:
        return tr("Keyboards");
    case DeviceCategory::MiceAndOtherPointingDevices:
        return tr("Mice and other pointing devices");
    case DeviceCategory::NetworkAdapters:
        return tr("Network adapters");
    case DeviceCategory::SoftwareDevices:
        return tr("Software devices");
    case DeviceCategory::SoundVideoAndGameControllers:
        return tr("Sound, video and game controllers");
    case DeviceCategory::StorageControllers:
        return tr("Storage controllers");
    case DeviceCategory::StorageVolumes:
        return tr("Storage volumes");
    case DeviceCategory::SystemDevices:
        return tr("System devices");
    case DeviceCategory::UniversalSerialBusControllers:
        return tr("Universal Serial Bus controllers");
    case DeviceCategory::Unknown:
    default:
        return deviceInfo_->subsystem();
    }
#elif defined(Q_OS_WIN)
    // Windows: Use pre-computed category from DeviceInfo (same as macOS)
    switch (deviceInfo_->category()) {
    case DeviceCategory::AudioInputsAndOutputs:
        return tr("Audio inputs and outputs");
    case DeviceCategory::Batteries:
        return tr("Batteries");
    case DeviceCategory::Computer:
        return tr("Computer");
    case DeviceCategory::DiskDrives:
        return tr("Disk drives");
    case DeviceCategory::DisplayAdapters:
        return tr("Display adapters");
    case DeviceCategory::DvdCdromDrives:
        return tr("DVD/CD-ROM drives");
    case DeviceCategory::HumanInterfaceDevices:
        return tr("Human Interface Devices");
    case DeviceCategory::Keyboards:
        return tr("Keyboards");
    case DeviceCategory::MiceAndOtherPointingDevices:
        return tr("Mice and other pointing devices");
    case DeviceCategory::NetworkAdapters:
        return tr("Network adapters");
    case DeviceCategory::SoftwareDevices:
        return tr("Software devices");
    case DeviceCategory::SoundVideoAndGameControllers:
        return tr("Sound, video and game controllers");
    case DeviceCategory::StorageControllers:
        return tr("Storage controllers");
    case DeviceCategory::StorageVolumes:
        return tr("Storage volumes");
    case DeviceCategory::SystemDevices:
        return tr("System devices");
    case DeviceCategory::UniversalSerialBusControllers:
        return tr("Universal Serial Bus controllers");
    case DeviceCategory::Unknown:
    default:
        return deviceInfo_->subsystem();
    }
#else
    namespace us = strings::udev;

    QString subsystem = deviceInfo_->subsystem();
    QString devpath = deviceInfo_->propertyValue(props::DEVPATH);
    QString pciClass = deviceInfo_->propertyValue(props::ID_PCI_CLASS_FROM_DATABASE);
    QString pciSubclass = deviceInfo_->propertyValue(props::ID_PCI_SUBCLASS_FROM_DATABASE);

    // Audio inputs and outputs
    if (pciSubclass == us::propertyValues::idPciSubclassFromDatabase::audioDevice()) {
        return tr("Audio inputs and outputs");
    }

    // Batteries
    if (deviceInfo_->propertyValue(props::ID_MODEL_FROM_DATABASE) ==
            us::propertyValues::idModelFromDatabase::ups() ||
        deviceInfo_->driver() == us::propertyValues::driver::battery()) {
        return tr("Batteries");
    }

    // Display adapters
    if (pciClass == us::propertyValues::idPciClassFromDatabase::displayController()) {
        return tr("Display adapters");
    }

    // USB controllers
    if (pciSubclass == us::propertyValues::idPciSubclassFromDatabase::usbController()) {
        return tr("Universal Serial Bus controllers");
    }

    // Storage controllers
    if (pciClass == us::propertyValues::idPciClassFromDatabase::massStorageController()) {
        return tr("Storage controllers");
    }

    // Network adapters
    if (pciClass == us::propertyValues::idPciClassFromDatabase::networkController()) {
        return tr("Network adapters");
    }

    // Block devices
    if (subsystem == us::subsystems::block()) {
        // CD-ROMs
        if (deviceInfo_->propertyValue(props::ID_CDROM) == strings::digit1()) {
            return tr("DVD/CD-ROM drives");
        }
        // Partitions (storage volumes)
        if (deviceInfo_->propertyValue(props::DEVTYPE) ==
            us::propertyValues::devType::partition()) {
            return tr("Storage volumes");
        }
        // Disk drives (non-virtual)
        if (!devpath.startsWith(QStringLiteral("/devices/virtual/"))) {
            return tr("Disk drives");
        }
    }

    // HID devices
    if (subsystem == us::subsystems::hid()) {
        return tr("Human Interface Devices");
    }

    // Keyboards
    if (!deviceInfo_->propertyValue(props::ID_INPUT_KEYBOARD).isEmpty()) {
        return tr("Keyboards");
    }

    // Mice
    if (!deviceInfo_->propertyValue(props::ID_INPUT_MOUSE).isEmpty()) {
        return tr("Mice and other pointing devices");
    }

    // Sound/video/game controllers
    if (deviceInfo_->propertyValue(props::ID_TYPE) == us::propertyValues::idType::audio()) {
        return tr("Sound, video and game controllers");
    }

    // Software devices (misc subsystem)
    if (subsystem == us::subsystems::misc()) {
        return tr("Software devices");
    }

    // System devices (PCI devices not categorized elsewhere)
    if (subsystem == us::subsystems::pci()) {
        return tr("System devices");
    }

    // Fallback to subsystem name
    if (!subsystem.isEmpty()) {
        return subsystem;
    }
#endif

    return tr("Unknown");
}

#ifdef Q_OS_LINUX
QString PropertiesDialog::getBlockDeviceManufacturer() {
    if (!deviceInfo_) {
        return {};
    }

    QString syspath = deviceInfo_->syspath();

    // Helper lambda to extract manufacturer from a model/name string
    auto extractManufacturer = [](const QString &text) -> QString {
        if (text.isEmpty()) {
            return {};
        }

        // Known manufacturer prefixes (case-insensitive matching)
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
            {QStringLiteral("CT"),
             QStringLiteral("Crucial")}, // Crucial product codes start with CT
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
            // Also check if it contains the manufacturer name
            if (text.contains(mapping.first, Qt::CaseInsensitive)) {
                return mapping.second;
            }
        }

        return {};
    };

    // First, try to extract from device name (most reliable for NVMe and many drives)
    QString deviceName = deviceInfo_->name();
    QString manufacturer = extractManufacturer(deviceName);
    if (!manufacturer.isEmpty()) {
        return manufacturer;
    }

    // Try ID_MODEL property
    QString model = deviceInfo_->propertyValue(props::ID_MODEL);
    manufacturer = extractManufacturer(model);
    if (!manufacturer.isEmpty()) {
        return manufacturer;
    }

    // Try ID_MODEL_FROM_DATABASE
    model = deviceInfo_->propertyValue(props::ID_MODEL_FROM_DATABASE);
    manufacturer = extractManufacturer(model);
    if (!manufacturer.isEmpty()) {
        return manufacturer;
    }

    // Try to read vendor from sysfs for block devices
    QString vendorPath = syspath + QStringLiteral("/device/vendor");
    QString vendor = safeReadSysfsFile(vendorPath);
    if (!vendor.isEmpty()) {
        vendor = vendor.trimmed();

        // Skip generic vendor strings
        if (vendor == QStringLiteral("ATA") || vendor == QStringLiteral("SATA") ||
            vendor == QStringLiteral("USB") || vendor == QStringLiteral("Generic") ||
            vendor == QStringLiteral("NVMe")) {
            // Already tried model extraction above
        } else {
            // Map common vendor strings
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

            // Check for WD prefix
            if (vendor.startsWith(QStringLiteral("WD"), Qt::CaseInsensitive)) {
                return QStringLiteral("Western Digital");
            }

            return vendor;
        }
    }

    // Try reading from parent device's vendor property for USB devices
    QString parentSyspath = deviceInfo_->parentSyspath();
    if (!parentSyspath.isEmpty()) {
        QString parentVendorPath = parentSyspath + QStringLiteral("/vendor");
        vendor = safeReadSysfsFile(parentVendorPath);
        if (!vendor.isEmpty()) {
            return vendor.trimmed();
        }
    }

    return {};
}
#else
QString PropertiesDialog::getBlockDeviceManufacturer() {
    return {};
}
#endif

#if defined(Q_OS_LINUX)
QString PropertiesDialog::getMountPoint() {
    if (!deviceInfo_) {
        return {};
    }

    QString devnode = deviceInfo_->devnode();
    // Fallback to DEVNAME property if devnode is empty
    if (devnode.isEmpty()) {
        devnode = deviceInfo_->propertyValue("DEVNAME");
    }
    if (devnode.isEmpty()) {
        return {};
    }

    // Get canonical path for the device node to handle symlinks
    QFileInfo devnodeInfo(devnode);
    QString canonicalDevnode = devnodeInfo.canonicalFilePath();
    if (canonicalDevnode.isEmpty()) {
        canonicalDevnode = devnode;
    }

    // Read /proc/mounts to find mount point for this device
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

            // Get canonical path for the mount device to handle symlinks
            QFileInfo mountDevInfo(device);
            QString canonicalMountDev = mountDevInfo.canonicalFilePath();
            if (canonicalMountDev.isEmpty()) {
                canonicalMountDev = device;
            }

            // Check if this is our device (compare both original and canonical paths)
            if (device == devnode || device == canonicalDevnode || canonicalMountDev == devnode ||
                canonicalMountDev == canonicalDevnode) {
                // Decode escaped characters in mount point (e.g., \040 for space)
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
#elif defined(Q_OS_MACOS)
#include <sys/mount.h>

QString PropertiesDialog::getMountPoint() {
    if (!deviceInfo_) {
        return {};
    }

    QString devnode = deviceInfo_->devnode();
    if (devnode.isEmpty()) {
        return {};
    }

    // Get canonical path for the device node
    QFileInfo devnodeInfo(devnode);
    QString canonicalDevnode = devnodeInfo.canonicalFilePath();
    if (canonicalDevnode.isEmpty()) {
        canonicalDevnode = devnode;
    }

    // Use getmntinfo to get mounted filesystems
    struct statfs *mounts;
    int numMounts = getmntinfo(&mounts, MNT_NOWAIT);

    for (int i = 0; i < numMounts; ++i) {
        QString mountDevice = QString::fromUtf8(mounts[i].f_mntfromname);
        QString mountPoint = QString::fromUtf8(mounts[i].f_mntonname);

        // Get canonical path for comparison
        QFileInfo mountDevInfo(mountDevice);
        QString canonicalMountDev = mountDevInfo.canonicalFilePath();
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
#elif defined(Q_OS_WIN)
QString PropertiesDialog::getMountPoint() {
    if (!deviceInfo_) {
        return {};
    }

    // On Windows, volumes have mount points like "C:\" or volume GUIDs
    // The device instance ID for volumes typically contains the volume GUID
    QString syspath = deviceInfo_->syspath();

    // Check if this is a volume device
    if (!syspath.contains(QStringLiteral("STORAGE\\VOLUME"), Qt::CaseInsensitive) &&
        !syspath.contains(QStringLiteral("Volume{"), Qt::CaseInsensitive)) {
        return {};
    }

    // Try to find the mount point using the device name
    QString devnode = deviceInfo_->devnode();
    if (devnode.isEmpty()) {
        devnode = deviceInfo_->name();
    }

    // For volumes with drive letters, return the drive letter
    wchar_t volumeName[MAX_PATH];
    wchar_t pathNames[MAX_PATH];
    DWORD charCount = MAX_PATH;

    HANDLE findHandle = FindFirstVolumeW(volumeName, MAX_PATH);
    if (findHandle == INVALID_HANDLE_VALUE) {
        return {};
    }

    QString result;
    do {
        // Remove trailing backslash for QueryDosDevice
        size_t len = wcslen(volumeName);
        if (len > 0 && volumeName[len - 1] == L'\\') {
            volumeName[len - 1] = L'\0';
        }

        // Get the mount points for this volume
        volumeName[len - 1] = L'\\';  // Restore backslash
        if (GetVolumePathNamesForVolumeNameW(volumeName, pathNames, charCount, &charCount)) {
            // pathNames is a multi-string
            if (pathNames[0] != L'\0') {
                result = QString::fromWCharArray(pathNames);
                // Check if this matches our device (simplified check)
                if (syspath.contains(QString::fromWCharArray(volumeName).mid(10, 38), Qt::CaseInsensitive)) {
                    FindVolumeClose(findHandle);
                    return result;
                }
            }
        }
    } while (FindNextVolumeW(findHandle, volumeName, MAX_PATH));

    FindVolumeClose(findHandle);
    return {};
}
#else
QString PropertiesDialog::getMountPoint() {
    return {};
}
#endif

void PropertiesDialog::populateDetailsTab() {
    if (!deviceInfo_)
        return;

    // Clear the combo box and rebuild with only properties that have values
    comboBoxDetailsProperty->clear();

    for (const PropertyMapping &mapping : propertyMappings_) {
        QString value;

        // Handle special properties that aren't direct udev lookups
        if (mapping.propertyKey == QStringLiteral("DEVICE_DESCRIPTION")) {
            value = deviceInfo_->name();
        } else if (mapping.propertyKey == QStringLiteral("SYSPATH")) {
            value = deviceInfo_->syspath();
        } else if (mapping.propertyKey == QStringLiteral("PARENT_SYSPATH")) {
            value = deviceInfo_->parentSyspath();
        } else if (mapping.propertyKey == QStringLiteral("CHILDREN")) {
            // Skip children for now - would require additional lookup
            continue;
        } else if (mapping.propertyKey == QStringLiteral("COMPATIBLE_IDS")) {
            // Skip for now
            continue;
        } else if (mapping.propertyKey == QStringLiteral("MOUNT_POINT")) {
            value = getMountPoint();
        } else {
            value = deviceInfo_->propertyValue(mapping.propertyKey.toLocal8Bit().constData());
        }

        // Only add property to dropdown if it has a value
        if (!value.isEmpty()) {
            comboBoxDetailsProperty->addItem(mapping.displayName, mapping.propertyKey);
        }
    }

    // Trigger initial selection
    if (comboBoxDetailsProperty->count() > 0) {
        onPropertySelectionChanged(0);
    }
}

void PropertiesDialog::onPropertySelectionChanged(int index) {
    listWidgetDetailsPropertyValue->clear();

    if (index < 0 || !deviceInfo_)
        return;

    QString propertyKey = comboBoxDetailsProperty->currentData().toString();
    QString value;

    // Handle special properties
    if (propertyKey == QStringLiteral("DEVICE_DESCRIPTION")) {
        value = deviceInfo_->name();
    } else if (propertyKey == QStringLiteral("SYSPATH")) {
        value = deviceInfo_->syspath();
    } else if (propertyKey == QStringLiteral("PARENT_SYSPATH")) {
        value = deviceInfo_->parentSyspath();
    } else if (propertyKey == QStringLiteral("MOUNT_POINT")) {
        value = getMountPoint();
    } else {
        value = deviceInfo_->propertyValue(propertyKey.toLocal8Bit().constData());
    }

    if (!value.isEmpty()) {
        // For multi-value properties, split by newline or semicolon
        if (value.contains(QStringLiteral("\n"))) {
            QStringList values = value.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
            for (const QString &v : values) {
                listWidgetDetailsPropertyValue->addItem(v.trimmed());
            }
        } else if (value.contains(QStringLiteral(";"))) {
            QStringList values = value.split(QStringLiteral(";"), Qt::SkipEmptyParts);
            for (const QString &v : values) {
                listWidgetDetailsPropertyValue->addItem(v.trimmed());
            }
        } else {
            listWidgetDetailsPropertyValue->addItem(value);
        }
    }
}

static void parseEventLine(const QString &event, QString &timestamp, QString &message) {
    timestamp.clear();
    message.clear();

#ifdef Q_OS_MACOS
    // macOS 'log show --style compact' format:
    // "YYYY-MM-DD HH:MM:SS.mmm Df subsystem[pid]: message"
    // or "YYYY-MM-DD HH:MM:SS.mmm Df category: message"
    QRegularExpression macLogRe(
        QStringLiteral("^(\\d{4}-\\d{2}-\\d{2}\\s+\\d{2}:\\d{2}:\\d{2}\\.\\d+)\\s+\\w+\\s+"));
    QRegularExpressionMatch match = macLogRe.match(event);

    if (match.hasMatch()) {
        QString dateTimeStr = match.captured(1);
        QString remainder = event.mid(match.capturedEnd());

        // Parse timestamp
        QDateTime dateTime = QDateTime::fromString(dateTimeStr, QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"));
        if (dateTime.isValid()) {
            timestamp = QLocale::system().toString(dateTime, QLocale::ShortFormat);
        } else {
            timestamp = dateTimeStr;
        }

        // Extract message after the process/subsystem name
        int colonIdx = remainder.indexOf(QStringLiteral(": "));
        if (colonIdx != -1) {
            message = remainder.mid(colonIdx + 2).trimmed();
        } else {
            message = remainder.trimmed();
        }
    } else {
        // Fallback for unexpected format
        message = event;
    }
#elif defined(Q_OS_WIN)
    // Windows wevtutil text output format varies by event type
    // Common patterns: "Event[...]: description" or "Date: YYYY-MM-DD HH:MM:SS"

    // Try to extract timestamp from "Date:" line or embedded timestamp
    QRegularExpression winDateRe(
        QStringLiteral("Date:\\s*(\\d{4}-\\d{2}-\\d{2}\\s+\\d{2}:\\d{2}:\\d{2})"));
    QRegularExpressionMatch match = winDateRe.match(event);

    if (match.hasMatch()) {
        QString dateTimeStr = match.captured(1);
        QDateTime dateTime = QDateTime::fromString(dateTimeStr, QStringLiteral("yyyy-MM-dd HH:mm:ss"));
        if (dateTime.isValid()) {
            timestamp = QLocale::system().toString(dateTime, QLocale::ShortFormat);
        } else {
            timestamp = dateTimeStr;
        }
        message = event;
    } else {
        // Check for message content
        QRegularExpression winMsgRe(QStringLiteral("(?:Message|Description):\\s*(.+)"));
        QRegularExpressionMatch msgMatch = winMsgRe.match(event);
        if (msgMatch.hasMatch()) {
            message = msgMatch.captured(1).trimmed();
        } else {
            message = event;
        }
    }
#else
    // Parse journalctl output with -o short-iso format
    // Format: "YYYY-MM-DDTHH:MM:SS+ZZZZ hostname kernel: message"

    // Find the ISO timestamp at the start (ends at first space after timezone)
    // Timezone can be +/-HH:MM or +/-HHMM format
    QRegularExpression isoTimestampRe(
        QStringLiteral("^(\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}[+-]\\d{2}:?\\d{2})\\s+"));
    QRegularExpressionMatch match = isoTimestampRe.match(event);

    if (match.hasMatch()) {
        QString isoTimestamp = match.captured(1);
        QString remainder = event.mid(match.capturedEnd());

        // Parse ISO timestamp and format according to user locale
        QDateTime dateTime = QDateTime::fromString(isoTimestamp, Qt::ISODate);
        if (dateTime.isValid()) {
            timestamp = QLocale::system().toString(dateTime, QLocale::ShortFormat);
        } else {
            timestamp = isoTimestamp;
        }

        // Extract message after "kernel:" or similar
        int kernelIdx = remainder.indexOf(QStringLiteral("kernel:"));
        if (kernelIdx != -1) {
            message = remainder.mid(kernelIdx + 7).trimmed();
        } else {
            int colonIdx = remainder.indexOf(QStringLiteral(": "));
            if (colonIdx != -1) {
                message = remainder.mid(colonIdx + 2).trimmed();
            } else {
                message = remainder.trimmed();
            }
        }
    } else {
        // Fallback for unexpected format
        message = event;
    }
#endif
}

void PropertiesDialog::populateEventsTab() {
    if (!deviceInfo_)
        return;

    eventsModel_->removeRows(0, eventsModel_->rowCount());
    textEditEventsInfo->setPlainText(tr("Loading events..."));
    buttonViewAllEvents->setEnabled(false);

    // Cancel any previous watcher before starting a new one
    if (eventsWatcher_) {
        eventsWatcher_->disconnect();
        eventsWatcher_->cancel();
        eventsWatcher_->waitForFinished();
        delete eventsWatcher_;
    }

    // Run getDeviceEvents() asynchronously to avoid blocking the UI
    eventsWatcher_ = new QFutureWatcher<QStringList>(this);
    connect(eventsWatcher_,
            &QFutureWatcher<QStringList>::finished,
            this,
            &PropertiesDialog::onEventsLoaded);

    // Capture device info for the lambda
    QString syspath = deviceInfo_->syspath();
    QString devnode = deviceInfo_->devnode();
    QString deviceName = deviceInfo_->name();
    QString vendorId = deviceInfo_->propertyValue("ID_VENDOR_ID");
    QString modelId = deviceInfo_->propertyValue("ID_MODEL_ID");

    auto future =
        QtConcurrent::run([syspath, devnode, deviceName, vendorId, modelId]() -> QStringList {
            QStringList events;

            // Build search terms - use specific identifiers only
            QStringList searchTerms;

            // Use USB vendor:product ID combination (most specific for USB devices)
            if (!vendorId.isEmpty() && !modelId.isEmpty()) {
                searchTerms << QStringLiteral("idVendor=%1").arg(vendorId.toLower());
            }

            // Use a significant part of the device name if available (at least 8 chars)
            if (!deviceName.isEmpty() && deviceName.length() >= 8) {
                QString nameSearch = deviceName.left(20).trimmed();
                int lastSpace = nameSearch.lastIndexOf(QLatin1Char(' '));
                if (lastSpace > 8) {
                    nameSearch = nameSearch.left(lastSpace);
                }
                searchTerms << nameSearch;
            }

            // Use device node name if available
            if (!devnode.isEmpty()) {
                QString shortName = devnode;
                if (shortName.startsWith(QStringLiteral("/dev/"))) {
                    shortName = shortName.mid(5);
                }
                if (shortName.length() >= 3) {
                    searchTerms << shortName;
                }
            }

            // Extract PCI address (Linux) or IORegistry path component (macOS)
            QRegularExpression pciRe(
                QStringLiteral("([0-9a-f]{4}:[0-9a-f]{2}:[0-9a-f]{2}\\.[0-9a-f])"));
            QRegularExpressionMatch pciMatch = pciRe.match(syspath);
            if (pciMatch.hasMatch()) {
                searchTerms << pciMatch.captured(1);
            }

            if (searchTerms.isEmpty()) {
                return events;
            }

#ifdef Q_OS_MACOS
            // macOS: Use 'log show' to query system logs
            QProcess logShow;
            QStringList args;
            args << QStringLiteral("show")
                 << QStringLiteral("--predicate")
                 << QStringLiteral("subsystem == 'com.apple.iokit' OR "
                                   "subsystem == 'com.apple.kernel' OR "
                                   "category == 'IOKit'")
                 << QStringLiteral("--last") << QStringLiteral("1h")
                 << QStringLiteral("--style") << QStringLiteral("compact");

            logShow.start(QStringLiteral("log"), args);
            if (logShow.waitForFinished(10000)) {
                QString output = QString::fromUtf8(logShow.readAllStandardOutput());
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
#elif defined(Q_OS_WIN)
            // Windows: Query Windows Event Log using wevtutil
            // We look for events from the System and Application logs
            QProcess wevtutil;
            QStringList args;
            // Query System log for device-related events
            args << QStringLiteral("qe") << QStringLiteral("System")
                 << QStringLiteral("/c:100")
                 << QStringLiteral("/rd:true")
                 << QStringLiteral("/f:text");

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
#else
            // Linux: Query journalctl for kernel messages about this device
            QProcess journalctl;
            QStringList args;
            args << QStringLiteral("-k") << QStringLiteral("-n") << QStringLiteral("500")
                 << QStringLiteral("--no-pager") << QStringLiteral("-o")
                 << QStringLiteral("short-iso");

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
#endif

            return events;
        });
    eventsWatcher_->setFuture(future);
}

void PropertiesDialog::onEventsLoaded() {
    if (!eventsWatcher_)
        return;

    allEvents_ = eventsWatcher_->result();
    delete eventsWatcher_;
    eventsWatcher_ = nullptr;

    if (allEvents_.isEmpty()) {
        textEditEventsInfo->setPlainText(tr("No events found."));
        buttonViewAllEvents->setEnabled(false);
        return;
    }

    // Show only first 5 events in the table
    int displayCount = qMin(5, allEvents_.size());
    for (int i = 0; i < displayCount; ++i) {
        QString timestamp, message;
        parseEventLine(allEvents_.at(i), timestamp, message);

        QList<QStandardItem *> row;
        row << new QStandardItem(timestamp);
        row << new QStandardItem(message);
        eventsModel_->appendRow(row);
    }

    // Enable "View All Events" button only if there are more than 5 events
    buttonViewAllEvents->setEnabled(allEvents_.size() > 5);

    // Resize columns
    tableViewEvents->resizeColumnsToContents();

    // Select the first event
    QModelIndex firstIndex = eventsModel_->index(0, 0);
    tableViewEvents->setCurrentIndex(firstIndex);
    onEventSelectionChanged(firstIndex, QModelIndex());
}

void PropertiesDialog::onEventSelectionChanged(const QModelIndex &current,
                                               const QModelIndex &previous) {
    Q_UNUSED(previous);

    if (!current.isValid()) {
        textEditEventsInfo->clear();
        return;
    }

    // Get the full message from the selected row
    int row = current.row();
    QStandardItem *timestampItem = eventsModel_->item(row, 0);
    QStandardItem *messageItem = eventsModel_->item(row, 1);

    if (timestampItem && messageItem) {
        QString info =
            QStringLiteral("%1\n\n%2").arg(timestampItem->text()).arg(messageItem->text());
        textEditEventsInfo->setPlainText(info);
    }
}

void PropertiesDialog::onDriverDetailsClicked() {
    if (!deviceInfo_)
        return;

    QString driver = deviceInfo_->driver();
    if (driver.isEmpty()) {
        return;
    }

    DriverDetailsDialog dialog(this);
    dialog.setCategoryIcon(categoryIcon_);
    dialog.setDriverName(driver);
    dialog.exec();
}

void PropertiesDialog::onViewAllEventsClicked() {
    if (allEvents_.isEmpty()) {
        return;
    }

    // Create a dialog to show all events
    QDialog dialog(this);
    dialog.setWindowTitle(tr("All Events - %1").arg(deviceInfo_ ? deviceInfo_->name() : QString()));
    dialog.resize(700, 500);

    auto *layout = new QVBoxLayout(&dialog);

    // Create table view for all events
    auto *tableView = new QTableView(&dialog);
    auto *model = new QStandardItemModel(&dialog);
    model->setHorizontalHeaderLabels({tr("Timestamp"), tr("Event")});

    for (const QString &event : allEvents_) {
        QString timestamp, message;
        parseEventLine(event, timestamp, message);

        QList<QStandardItem *> row;
        row << new QStandardItem(timestamp);
        row << new QStandardItem(message);
        model->appendRow(row);
    }

    tableView->setModel(model);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->resizeColumnsToContents();

    layout->addWidget(tableView);

    // Add close button
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    dialog.exec();
}

void PropertiesDialog::onCopyDevicePath() {
    // Only copy when General tab is active
    if (tabWidget->currentWidget() != tabGeneral) {
        return;
    }

    if (!syspath_.isEmpty()) {
        QApplication::clipboard()->setText(syspath_);
    }
}

#ifdef Q_OS_LINUX
namespace {
    // Check if path is a PCI device sysfs path
    bool isPciDevice(const QString &syspath) {
        // PCI device paths contain /pci and have format like 0000:00:1f.0
        static const QRegularExpression pciPathRe(QStringLiteral(
            "/pci[^/]*/[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\\.[0-9a-fA-F]$"));
        return pciPathRe.match(syspath).hasMatch();
    }
} // namespace

QList<PropertiesDialog::ResourceInfo> PropertiesDialog::getDeviceResources() {
    QList<ResourceInfo> resources;

    if (!deviceInfo_) {
        return resources;
    }

    QString syspath = deviceInfo_->syspath();

    // Only read resources for PCI devices - other device types may have
    // sysfs attributes that block or require special permissions
    if (!isPciDevice(syspath)) {
        return resources;
    }

    // Get IRQ from sysfs
    QString irqPath = syspath + QStringLiteral("/irq");
    QString irq = safeReadSysfsFile(irqPath);
    if (!irq.isEmpty() && irq != QStringLiteral("0")) {
        int irqNum = irq.toInt();
        QString setting =
            QStringLiteral("0x%1 (%2)").arg(irqNum, 8, 16, QLatin1Char('0')).arg(irqNum).toUpper();
        resources.append({tr("IRQ"), setting, QStringLiteral("preferences-other")});
    }

    // Get PCI resources (memory ranges and I/O ports)
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

            // Format: start end flags
            QStringList parts = trimmedLine.split(whitespaceRe, Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                bool ok1, ok2, ok3;
                qulonglong start = parts[0].toULongLong(&ok1, 16);
                qulonglong end = parts[1].toULongLong(&ok2, 16);
                qulonglong flags = parts[2].toULongLong(&ok3, 16);

                if (ok1 && ok2 && ok3 && start != 0 && end != 0) {
                    // Check flags to determine resource type
                    // IORESOURCE_IO = 0x00000100, IORESOURCE_MEM = 0x00000200
                    QString type;
                    if (flags & 0x00000100) {
                        type = tr("I/O Range");
                    } else if (flags & 0x00000200) {
                        type = tr("Memory Range");
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

    // Get DMA if available (rare on modern systems)
    QString driverName = deviceInfo_->driver();
    if (!driverName.isEmpty()) {
        QString dmaContent = safeReadSysfsFile(QStringLiteral("/proc/dma"));
        if (!dmaContent.isEmpty()) {
            static const QRegularExpression dmaRe(QStringLiteral("^(\\d+):\\s*(.*)$"));
            const QStringList lines = dmaContent.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                if (line.contains(driverName, Qt::CaseInsensitive)) {
                    auto match = dmaRe.match(line);
                    if (match.hasMatch()) {
                        QString channel = match.captured(1);
                        resources.append({tr("DMA"), channel, QStringLiteral("preferences-other")});
                    }
                }
            }
        }
    }

    return resources;
}

void PropertiesDialog::createResourcesTab() {
    // Remove existing resources tab if present
    if (resourcesTab_) {
        int index = tabWidget->indexOf(resourcesTab_);
        if (index >= 0) {
            tabWidget->removeTab(index);
        }
        delete resourcesTab_;
        resourcesTab_ = nullptr;
    }

    QList<ResourceInfo> resources = getDeviceResources();

    // Only create the tab if there are resources
    if (resources.isEmpty()) {
        return;
    }

    // Create the Resources tab widget
    resourcesTab_ = new QWidget();

    auto *mainLayout = new QVBoxLayout(resourcesTab_);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Top section: icon and device name
    auto *topLayout = new QHBoxLayout();
    auto *iconLabel = new QLabel(resourcesTab_);
    iconLabel->setFixedSize(32, 32);
    iconLabel->setPixmap(categoryIcon_.pixmap(32, 32));
    topLayout->addWidget(iconLabel);

    topLayout->addSpacing(15);

    auto *nameLabel = new QLabel(deviceInfo_->name(), resourcesTab_);
    topLayout->addWidget(nameLabel, 1);
    mainLayout->addLayout(topLayout);

    mainLayout->addSpacing(10);

    // Resource settings label
    auto *settingsLabel = new QLabel(tr("Resource settings:"), resourcesTab_);
    mainLayout->addWidget(settingsLabel);

    // Resource tree widget
    auto *resourceTree = new QTreeWidget(resourcesTab_);
    resourceTree->setHeaderLabels({tr("Resource type"), tr("Setting")});
    resourceTree->setRootIsDecorated(false);
    resourceTree->setAlternatingRowColors(true);

    for (const ResourceInfo &res : resources) {
        auto *item = new QTreeWidgetItem(resourceTree);
        item->setIcon(0, QIcon::fromTheme(res.iconName));
        item->setText(0, res.type);
        item->setText(1, res.setting);
    }

    resourceTree->resizeColumnToContents(0);
    mainLayout->addWidget(resourceTree);

    mainLayout->addSpacing(10);

    // Settings based on (disabled)
    auto *settingsBasedLayout = new QHBoxLayout();
    auto *settingsBasedLabel = new QLabel(tr("Setting based on:"), resourcesTab_);
    settingsBasedLayout->addWidget(settingsBasedLabel);

    auto *settingsBasedCombo = new QComboBox(resourcesTab_);
    settingsBasedCombo->addItem(tr("Current configuration"));
    settingsBasedCombo->setEnabled(false);
    settingsBasedLayout->addWidget(settingsBasedCombo, 1);
    mainLayout->addLayout(settingsBasedLayout);

    mainLayout->addSpacing(5);

    // Use automatic settings checkbox and Change Setting button
    auto *autoSettingsLayout = new QHBoxLayout();
    auto *autoSettingsCheck = new QCheckBox(tr("Use automatic settings"), resourcesTab_);
    autoSettingsCheck->setChecked(true);
    autoSettingsCheck->setEnabled(false);
    autoSettingsLayout->addWidget(autoSettingsCheck);

    autoSettingsLayout->addStretch();

    auto *changeSettingBtn = new QPushButton(tr("Change Setting..."), resourcesTab_);
    changeSettingBtn->setEnabled(false);
    autoSettingsLayout->addWidget(changeSettingBtn);
    mainLayout->addLayout(autoSettingsLayout);

    mainLayout->addSpacing(10);

    // Conflicting device list group
    auto *conflictGroup = new QGroupBox(tr("Conflicting device list:"), resourcesTab_);
    auto *conflictLayout = new QVBoxLayout(conflictGroup);

    auto *conflictList = new QListWidget(conflictGroup);
    conflictList->addItem(tr("No conflicts."));
    conflictList->setEnabled(false);
    conflictLayout->addWidget(conflictList);

    mainLayout->addWidget(conflictGroup);

    // Add the Resources tab at the end
    tabWidget->addTab(resourcesTab_, tr("Resources"));
}
#else
QList<PropertiesDialog::ResourceInfo> PropertiesDialog::getDeviceResources() {
    return {};
}

void PropertiesDialog::createResourcesTab() {
    // Resources tab not available on non-Linux platforms
}
#endif
