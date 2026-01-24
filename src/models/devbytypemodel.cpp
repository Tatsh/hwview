#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>
#ifdef Q_OS_MACOS
#include <QtCore/QProcess>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "const_strings.h"
#include "devicecache.h"
#include "models/devbytypemodel.h"

namespace s = strings;
namespace us = strings::udev;

DevicesByTypeModel::DevicesByTypeModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr), audioInputsAndOutputsItem(nullptr),
      batteriesItem(nullptr), computerItem(nullptr), diskDrivesItem(nullptr),
      displayAdaptersItem(nullptr), dvdCdromDrivesItem(nullptr), humanInterfaceDevicesItem(nullptr),
      keyboardsItem(nullptr), miceAndOtherPointingDevicesItem(nullptr),
      networkAdaptersItem(nullptr), softwareDevicesItem(nullptr),
      soundVideoAndGameControllersItem(nullptr), storageControllersItem(nullptr),
      storageVolumesItem(nullptr), systemDevicesItem(nullptr),
      universalSerialBusControllersItem(nullptr) {
    auto *root = new Node({s::empty(), s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname(), s::empty()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

QVariant DevicesByTypeModel::decorationData(Node *item, int column) const {
    // Only show icons in the first column (Name)
    if (column != 0) {
        return {};
    }
    if (item->isHidden()) {
        // Return cached greyed-out pixmap for hidden devices
        return item->disabledPixmap(16);
    }
    return item->icon();
}

void DevicesByTypeModel::finalizeCategory(Node *&category) {
    if (category && category->childCount() > 0) {
        category->sortChildren();
        hostnameItem->appendChild(category);
    } else if (category) {
        delete category;
        category = nullptr;
    }
}

void DevicesByTypeModel::buildTree() {
    // Create all category nodes upfront (with two columns for Name and Driver)
    audioInputsAndOutputsItem =
        new Node({tr("Audio inputs and outputs"), s::empty()}, hostnameItem);
    audioInputsAndOutputsItem->setIcon(s::categoryIcons::audioInputs());

    batteriesItem = new Node({tr("Batteries"), s::empty()}, hostnameItem);
    batteriesItem->setIcon(s::categoryIcons::batteries());

    computerItem = new Node({tr("Computer"), s::empty()}, hostnameItem);
    computerItem->setIcon(s::categoryIcons::computer());

    diskDrivesItem = new Node({tr("Disk drives"), s::empty()}, hostnameItem);
    diskDrivesItem->setIcon(s::categoryIcons::diskDrives());

    displayAdaptersItem = new Node({tr("Display adapters"), s::empty()}, hostnameItem);
    displayAdaptersItem->setIcon(s::categoryIcons::displayAdapters());

    dvdCdromDrivesItem = new Node({tr("DVD/CD-ROM drives"), s::empty()}, hostnameItem);
    dvdCdromDrivesItem->setIcon(s::categoryIcons::dvdCdromDrives());

    humanInterfaceDevicesItem = new Node({tr("Human Interface Devices"), s::empty()}, hostnameItem);
    humanInterfaceDevicesItem->setIcon(s::categoryIcons::hid());

    keyboardsItem = new Node({tr("Keyboards"), s::empty()}, hostnameItem);
    keyboardsItem->setIcon(s::categoryIcons::keyboards());

    miceAndOtherPointingDevicesItem =
        new Node({tr("Mice and other pointing devices"), s::empty()}, hostnameItem);
    miceAndOtherPointingDevicesItem->setIcon(s::categoryIcons::mice());

    networkAdaptersItem = new Node({tr("Network adapters"), s::empty()}, hostnameItem);
    networkAdaptersItem->setIcon(s::categoryIcons::networkAdapters());

    softwareDevicesItem = new Node({tr("Software devices"), s::empty()}, hostnameItem);
    softwareDevicesItem->setIcon(s::categoryIcons::other());

    soundVideoAndGameControllersItem =
        new Node({tr("Sound, video and game controllers"), s::empty()}, hostnameItem);
    soundVideoAndGameControllersItem->setIcon(s::categoryIcons::soundVideoGameControllers());

    storageControllersItem = new Node({tr("Storage controllers"), s::empty()}, hostnameItem);
    storageControllersItem->setIcon(s::categoryIcons::storageControllers());

    storageVolumesItem = new Node({tr("Storage volumes"), s::empty()}, hostnameItem);
    storageVolumesItem->setIcon(s::categoryIcons::storageVolumes());

    systemDevicesItem = new Node({tr("System devices"), s::empty()}, hostnameItem);
    systemDevicesItem->setIcon(s::categoryIcons::systemDevices());

    universalSerialBusControllersItem =
        new Node({tr("Universal Serial Bus controllers"), s::empty()}, hostnameItem);
    universalSerialBusControllersItem->setIcon(s::categoryIcons::usbControllers());

    // Add computer info - determine system type based on firmware
    QString computerName;
    QString computerSyspath;
#ifdef Q_OS_LINUX
    QFileInfo acpiInfo(QStringLiteral("/sys/firmware/acpi"));
    QFileInfo dtInfo(QStringLiteral("/sys/firmware/devicetree"));

    if (acpiInfo.exists() && acpiInfo.isDir()) {
#if defined(Q_PROCESSOR_X86_64)
        computerName = tr("ACPI x64-based PC");
#elif defined(Q_PROCESSOR_X86_32)
        computerName = tr("ACPI x86-based PC");
#elif defined(Q_PROCESSOR_ARM_64)
        computerName = tr("ACPI ARM64-based PC");
#elif defined(Q_PROCESSOR_ARM)
        computerName = tr("ACPI ARM-based PC");
#else
        computerName = tr("ACPI-based PC");
#endif
    } else if (dtInfo.exists() && dtInfo.isDir()) {
        computerName = tr("Device Tree-based System");
    } else {
        computerName = tr("Standard PC");
    }
    computerSyspath = QStringLiteral("/sys/devices/virtual/dmi/id");
#elif defined(Q_OS_MACOS)
    // macOS: Detect Mac model
#if defined(Q_PROCESSOR_ARM_64)
    computerName = tr("Apple Silicon Mac");
#elif defined(Q_PROCESSOR_X86_64)
    computerName = tr("Intel-based Mac");
#else
    computerName = tr("Mac");
#endif
    // Try to get more specific model name using sysctl
    QProcess sysctl;
    sysctl.start(QStringLiteral("sysctl"), {QStringLiteral("-n"), QStringLiteral("hw.model")});
    if (sysctl.waitForFinished(1000)) {
        QString model = QString::fromUtf8(sysctl.readAllStandardOutput()).trimmed();
        if (!model.isEmpty()) {
            // Convert model identifier to friendly name
            if (model.startsWith(QStringLiteral("Mac"))) {
                if (model.contains(QStringLiteral("BookPro"))) {
                    computerName = tr("MacBook Pro");
                } else if (model.contains(QStringLiteral("BookAir"))) {
                    computerName = tr("MacBook Air");
                } else if (model.contains(QStringLiteral("Book"))) {
                    computerName = tr("MacBook");
                } else if (model.contains(QStringLiteral("Pro"))) {
                    computerName = tr("Mac Pro");
                } else if (model.contains(QStringLiteral("mini"))) {
                    computerName = tr("Mac mini");
                } else if (model.contains(QStringLiteral("Studio"))) {
                    computerName = tr("Mac Studio");
                } else {
                    computerName = tr("Mac (%1)").arg(model);
                }
            } else if (model.startsWith(QStringLiteral("iMac"))) {
                computerName = tr("iMac");
            } else {
                computerName = model;
            }
        }
    }
    computerSyspath = QStringLiteral("IOService:/");
#elif defined(Q_OS_WIN)
    // Windows: Detect system type using GetNativeSystemInfo
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        computerName = tr("x64-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        computerName = tr("ARM64-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        computerName = tr("x86-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        computerName = tr("ARM-based PC");
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        computerName = tr("Itanium-based PC");
        break;
    default:
        computerName = tr("Standard PC");
        break;
    }

    // Check for ACPI (all modern Windows PCs are ACPI)
    computerName = tr("ACPI %1").arg(computerName);
    computerSyspath = QStringLiteral("ACPI_HAL\\PNP0C08\\0");
#else
    computerName = tr("Standard PC");
#endif

    auto *acpiNode = new Node({computerName, s::empty()}, computerItem, NodeType::Device);
    if (!computerSyspath.isEmpty()) {
        acpiNode->setSyspath(computerSyspath);
    }
    acpiNode->setIcon(computerItem->icon());
    computerItem->appendChild(acpiNode);

    static const QRegularExpression kBeginningWithSlashDevRe(QStringLiteral("^/dev/"));

    bool showHidden = DeviceCache::instance().showHiddenDevices();

    // Single pass through all cached devices - use pre-computed category for fast classification
    for (const DeviceInfo &info : DeviceCache::instance().allDevices()) {
        // Skip hidden devices unless show hidden is enabled
        if (info.isHidden() && !showHidden) {
            continue;
        }

        Node *parentNode = nullptr;
        QString rawName = info.name();
        QString displayName = rawName;

        // Use pre-computed category for O(1) classification
        switch (info.category()) {
        case DeviceCategory::AudioInputsAndOutputs:
            parentNode = audioInputsAndOutputsItem;
            break;

        case DeviceCategory::Batteries:
            displayName = s::acpiDeviceNiceName(info.devPath(), rawName);
            parentNode = batteriesItem;
            break;

        case DeviceCategory::DisplayAdapters:
            parentNode = displayAdaptersItem;
            break;

        case DeviceCategory::UniversalSerialBusControllers:
            parentNode = universalSerialBusControllersItem;
            break;

        case DeviceCategory::StorageControllers:
            parentNode = storageControllersItem;
            break;

        case DeviceCategory::NetworkAdapters:
            parentNode = networkAdaptersItem;
            break;

        case DeviceCategory::DvdCdromDrives:
            parentNode = dvdCdromDrivesItem;
            break;

        case DeviceCategory::StorageVolumes: {
            // Try to get a nice name: partition label, filesystem label, or device name
            QString volumeName = info.propertyValue(us::propertyNames::ID_PART_ENTRY_NAME);
            if (volumeName.isEmpty()) {
                volumeName = info.propertyValue(us::propertyNames::ID_FS_LABEL);
            }
            if (!volumeName.isEmpty()) {
                displayName = volumeName;
            }
            parentNode = storageVolumesItem;
            break;
        }

        case DeviceCategory::DiskDrives:
            parentNode = diskDrivesItem;
            break;

        case DeviceCategory::HumanInterfaceDevices:
            displayName = s::softwareDeviceNiceName(rawName);
            parentNode = humanInterfaceDevicesItem;
            break;

        case DeviceCategory::Keyboards:
            displayName = s::softwareDeviceNiceName(rawName);
            parentNode = keyboardsItem;
            break;

        case DeviceCategory::MiceAndOtherPointingDevices:
            displayName = s::softwareDeviceNiceName(rawName);
            parentNode = miceAndOtherPointingDevicesItem;
            break;

        case DeviceCategory::SoundVideoAndGameControllers:
            parentNode = soundVideoAndGameControllersItem;
            break;

        case DeviceCategory::SoftwareDevices: {
            rawName = info.name();
            rawName.replace(kBeginningWithSlashDevRe, s::empty());
            displayName = s::softwareDeviceNiceName(rawName);
            parentNode = softwareDevicesItem;
            break;
        }

        case DeviceCategory::SystemDevices:
            parentNode = systemDevicesItem;
            break;

        case DeviceCategory::Computer:
        case DeviceCategory::Unknown:
        default:
            // Skip unknown devices
            continue;
        }

        if (parentNode) {
            auto *node = new Node({displayName, info.driver()}, parentNode, NodeType::Device);
            node->setSyspath(info.syspath());
            node->setIsHidden(info.isHidden());
            node->setRawName(rawName);
            node->setIcon(parentNode->icon());
            parentNode->appendChild(node);
        }
    }

    // Finalize categories - only add non-empty ones
    finalizeCategory(audioInputsAndOutputsItem);
    finalizeCategory(batteriesItem);
    finalizeCategory(computerItem);
    finalizeCategory(diskDrivesItem);
    finalizeCategory(displayAdaptersItem);
    finalizeCategory(dvdCdromDrivesItem);
    finalizeCategory(humanInterfaceDevicesItem);
    finalizeCategory(keyboardsItem);
    finalizeCategory(miceAndOtherPointingDevicesItem);
    finalizeCategory(networkAdaptersItem);
    finalizeCategory(softwareDevicesItem);
    finalizeCategory(soundVideoAndGameControllersItem);
    finalizeCategory(storageControllersItem);
    finalizeCategory(storageVolumesItem);
    finalizeCategory(systemDevicesItem);
    finalizeCategory(universalSerialBusControllersItem);

    // Sort categories alphabetically
    hostnameItem->sortChildren();
}

QVariant DevicesByTypeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        } else if (section == 1) {
            return tr("Driver");
        }
    }
    return {};
}
