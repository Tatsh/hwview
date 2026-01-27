// SPDX-License-Identifier: MIT
#include <QtCore/QMap>
#include <QtCore/QSet>

#include "const_strings.h"
#include "devicecache.h"
#include "models/drvbytypemodel.h"

namespace s = strings;
namespace us = strings::udev;

DriversByTypeModel::DriversByTypeModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr) {
    auto *root = new Node({s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

void DriversByTypeModel::finalizeCategory(Node *&category) {
    if (category && category->childCount() > 0) {
        hostnameItem->appendChild(category);
    } else if (category) {
        delete category;
        category = nullptr;
    }
}

void DriversByTypeModel::buildTree() {
    // Collect unique drivers by category
    QMap<QString, QSet<QString>> driversByCategory;
    auto showHidden = DeviceCache::instance().showHiddenDevices();

    for (const DeviceInfo &info : DeviceCache::instance().allDevices()) {
        // Skip hidden devices unless show hidden is enabled
        if (info.isHidden() && !showHidden) {
            continue;
        }

        auto driver = info.driver();
        if (driver.isEmpty()) {
            continue;
        }

        QString category;

        // Use pre-computed category for O(1) classification
        switch (info.category()) {
        case DeviceCategory::AudioInputsAndOutputs:
        case DeviceCategory::SoundVideoAndGameControllers:
            category = tr("Sound, video and game controllers");
            break;
        case DeviceCategory::DisplayAdapters:
            category = tr("Display adapters");
            break;
        case DeviceCategory::NetworkAdapters:
            category = tr("Network adapters");
            break;
        case DeviceCategory::StorageControllers:
            category = tr("Storage controllers");
            break;
        case DeviceCategory::UniversalSerialBusControllers:
            category = tr("Universal Serial Bus controllers");
            break;
        case DeviceCategory::DiskDrives:
        case DeviceCategory::DvdCdromDrives:
        case DeviceCategory::StorageVolumes:
            category = tr("Disk drives");
            break;
        case DeviceCategory::HumanInterfaceDevices:
            category = tr("Human Interface Devices");
            break;
        case DeviceCategory::Keyboards:
            category = tr("Keyboards");
            break;
        case DeviceCategory::MiceAndOtherPointingDevices:
            category = tr("Mice and other pointing devices");
            break;
        case DeviceCategory::Batteries:
            category = tr("Batteries");
            break;
        case DeviceCategory::SystemDevices:
            category = tr("System devices");
            break;
        case DeviceCategory::SoftwareDevices:
            category = tr("Software devices");
            break;
        case DeviceCategory::Computer:
        case DeviceCategory::Unknown:
        default:
            category = tr("Other devices");
            break;
        }

        if (!category.isEmpty()) {
            driversByCategory[category].insert(driver);
        }
    }

    // Create nodes for each category and its drivers
    for (auto it = driversByCategory.constBegin(); it != driversByCategory.constEnd(); ++it) {
        const QString &categoryName = it.key();
        const QSet<QString> &drivers = it.value();

        auto *categoryNode = new Node({categoryName}, hostnameItem);
        categoryNode->setIcon(s::categoryIcons::forCategory(categoryName));

        // Sort drivers
        auto sortedDrivers = drivers.values();
        std::sort(sortedDrivers.begin(), sortedDrivers.end());

        for (const QString &driver : sortedDrivers) {
            auto *driverNode = new Node({driver}, categoryNode, NodeType::Label);
            driverNode->setIcon(s::categoryIcons::forDriver(driver));
            categoryNode->appendChild(driverNode);
        }

        finalizeCategory(categoryNode);
    }
}

QVariant DriversByTypeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        }
    }
    return {};
}
