#include "viewerdevbydrivermodel.h"

#include "const_strings.h"
#include "exporteddata.h"
#include "models/node.h"

#include <QMap>

namespace s = strings;

ViewerDevicesByDriverModel::ViewerDevicesByDriverModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr) {
    auto *root = new Node({s::empty()});
    setRootItem(root);
    hostnameItem = new Node({ExportedData::instance().hostname()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

void ViewerDevicesByDriverModel::buildTree() {
    auto &data = ExportedData::instance();
    const auto &allDevices = data.allDevices();
    bool showHidden = data.showHiddenDevices();

    // Map from driver name to list of device indices
    QMap<QString, QVector<int>> devicesByDriver;

    for (int i = 0; i < allDevices.size(); ++i) {
        const ExportedData::Device &dev = allDevices.at(i);
        // Skip hidden devices if not showing them
        if (dev.isHidden && !showHidden) {
            continue;
        }
        if (!dev.isValidForDisplay) {
            continue;
        }

        QString driver = dev.driver;
        if (driver.isEmpty()) {
            driver = tr("(No driver)");
        }

        devicesByDriver[driver].append(i);
    }

    // Create nodes for each driver and its devices
    for (auto it = devicesByDriver.constBegin(); it != devicesByDriver.constEnd(); ++it) {
        const QString &driverName = it.key();
        const QVector<int> &deviceIndices = it.value();

        auto *driverNode = new Node({driverName}, hostnameItem);
        driverNode->setIcon(s::categoryIcons::forDriver(driverName));
        hostnameItem->appendChild(driverNode);

        // Sort by name
        QVector<int> sortedIndices = deviceIndices;
        std::sort(sortedIndices.begin(), sortedIndices.end(), [&allDevices](int a, int b) {
            return allDevices.at(a).name < allDevices.at(b).name;
        });

        for (int idx : sortedIndices) {
            const ExportedData::Device &dev = allDevices.at(idx);
            QString rawName = dev.name;
            if (rawName.isEmpty()) {
                auto lastSlash = dev.syspath.lastIndexOf(QLatin1Char('/'));
                if (lastSlash >= 0) {
                    rawName = dev.syspath.mid(lastSlash + 1);
                } else {
                    rawName = dev.syspath;
                }
            }
            QString name = s::softwareDeviceNiceName(rawName);

            auto *deviceNode = new Node({name}, driverNode, NodeType::Device);
            deviceNode->setSyspath(dev.syspath);
            deviceNode->setIsHidden(dev.isHidden);
            deviceNode->setRawName(rawName);
            deviceNode->setIcon(s::categoryIcons::forSubsystem(dev.subsystem));
            driverNode->appendChild(deviceNode);
        }
    }
}

int ViewerDevicesByDriverModel::columnCount([[maybe_unused]] const QModelIndex &parent) const {
    return 1;
}

QVariant ViewerDevicesByDriverModel::headerData(int section,
                                                Qt::Orientation orientation,
                                                int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        }
    }
    return {};
}
