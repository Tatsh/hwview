#include <QtCore/QMap>

#include "const_strings.h"
#include "devicecache.h"
#include "models/devbydrivermodel.h"

namespace s = strings;

DevicesByDriverModel::DevicesByDriverModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr) {
    auto *root = new Node({s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

void DevicesByDriverModel::buildTree() {
    // Store device list locally to avoid dangling pointers
    QList<DeviceInfo> allDevices = DeviceCache::instance().allDevices();

    // Map from driver name to list of device indices
    QMap<QString, QVector<int>> devicesByDriver;
    bool showHidden = DeviceCache::instance().showHiddenDevices();

    // Build index by driver
    for (int i = 0; i < allDevices.size(); ++i) {
        const DeviceInfo &info = allDevices.at(i);
        // Skip hidden devices unless show hidden is enabled
        if (info.isHidden() && !showHidden) {
            continue;
        }
        if (!info.isValidForDisplay()) {
            continue;
        }

        QString driver = info.driver();
        if (driver.isEmpty()) {
            driver = tr("(No driver)");
        }

        devicesByDriver[driver].append(i);
    }

    // Create nodes for each driver and its devices
    for (auto it = devicesByDriver.constBegin(); it != devicesByDriver.constEnd(); ++it) {
        const QString &driverName = it.key();
        const QVector<int> &deviceIndices = it.value();

        // Create driver category node
        auto *driverNode = new Node({driverName}, hostnameItem);
        driverNode->setIcon(s::categoryIcons::forDriver(driverName));
        hostnameItem->appendChild(driverNode);

        // Sort device indices by name
        QVector<int> sortedIndices = deviceIndices;
        std::sort(sortedIndices.begin(), sortedIndices.end(), [&allDevices](int a, int b) {
            return allDevices.at(a).name() < allDevices.at(b).name();
        });

        // Add devices under this driver
        for (int idx : sortedIndices) {
            const DeviceInfo &info = allDevices.at(idx);
            QString rawName = info.name();
            if (rawName.isEmpty()) {
                // Fall back to syspath basename
                const QString &syspath = info.syspath();
                auto lastSlash = syspath.lastIndexOf(QLatin1Char('/'));
                if (lastSlash >= 0) {
                    rawName = syspath.mid(lastSlash + 1);
                } else {
                    rawName = syspath;
                }
            }
            // Convert to display name if available
            QString name = s::softwareDeviceDisplayName(rawName);

            auto *deviceNode = new Node({name}, driverNode, NodeType::Device);
            deviceNode->setSyspath(info.syspath());
            deviceNode->setIsHidden(info.isHidden());
            deviceNode->setRawName(rawName);
            deviceNode->setIcon(s::categoryIcons::forSubsystem(info.subsystem()));
            driverNode->appendChild(deviceNode);
        }
    }
}

QVariant
DevicesByDriverModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        }
    }
    return {};
}
