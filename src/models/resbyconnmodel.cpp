// SPDX-License-Identifier: MIT
#include <QtCore/QStack>

#include "const_strings.h"
#include "devicecache.h"
#include "models/resbyconnmodel.h"
#include "systeminfo.h"
#include "viewsettings.h"

namespace s = strings;

ResourcesByConnectionModel::ResourcesByConnectionModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr), dmaItem(nullptr), ioItem(nullptr),
      irqItem(nullptr), memoryItem(nullptr) {
    auto *root = new Node({s::empty(), s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname(), s::empty()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

bool ResourcesByConnectionModel::shouldShowIcons() const {
    return ViewSettings::instance().showDeviceIcons();
}

void ResourcesByConnectionModel::buildTree() {
    addDma();
    addIoPorts();
    addIrq();
    addMemory();
}

void ResourcesByConnectionModel::addDma() {
    auto channels = getSystemDmaChannels();
    if (channels.isEmpty()) {
        return;
    }

    dmaItem = new Node({tr("Direct memory access (DMA)"), s::empty()}, hostnameItem);
    dmaItem->setIcon(s::categoryIcons::dma());

    for (const auto &channel : channels) {
        auto displayText = QStringLiteral("[%1] %2").arg(channel.channel, channel.name);
        auto *node = new Node({displayText, s::empty()}, dmaItem);
        node->setIcon(s::categoryIcons::dma());
        dmaItem->appendChild(node);
    }

    hostnameItem->appendChild(dmaItem);
}

void ResourcesByConnectionModel::buildHierarchicalResource([[maybe_unused]] Node *categoryNode,
                                                           const QIcon &itemIcon,
                                                           int indentLevel,
                                                           const QString &rangeStart,
                                                           const QString &rangeEnd,
                                                           const QString &name,
                                                           QStack<QPair<int, Node *>> &nodeStack) {
    // Pop stack until we find a parent with smaller indentation
    while (nodeStack.size() > 1 && nodeStack.top().first >= indentLevel) {
        nodeStack.pop();
    }

    auto *parentNode = nodeStack.top().second;

    auto displayText = QStringLiteral("[%1 - %2] %3").arg(rangeStart, rangeEnd, name);
    auto *node = new Node({displayText, s::empty()}, parentNode);
    node->setIcon(itemIcon);
    parentNode->appendChild(node);

    // Push this node as potential parent for more indented entries
    nodeStack.push({indentLevel, node});
}

void ResourcesByConnectionModel::addIoPorts() {
    auto ports = getSystemIoPorts();
    if (ports.isEmpty()) {
        return;
    }

    ioItem = new Node({tr("Input/output (IO)"), s::empty()}, hostnameItem);
    ioItem->setIcon(s::categoryIcons::ioPorts());

    QStack<QPair<int, Node *>> nodeStack;
    nodeStack.push({-1, ioItem});

    for (const auto &port : ports) {
        buildHierarchicalResource(ioItem,
                                  s::categoryIcons::ioPorts(),
                                  port.indentLevel,
                                  port.rangeStart,
                                  port.rangeEnd,
                                  port.name,
                                  nodeStack);
    }

    hostnameItem->appendChild(ioItem);
}

void ResourcesByConnectionModel::addIrq() {
    auto irqs = getSystemIrqs();
    if (irqs.isEmpty()) {
        return;
    }

    irqItem = new Node({tr("Interrupt request (IRQ)"), s::empty()}, hostnameItem);
    irqItem->setIcon(s::categoryIcons::irq());

    for (const auto &irq : irqs) {
        QString displayText;
        if (!irq.irqType.isEmpty()) {
            displayText =
                QStringLiteral("(%1) %2 %3").arg(irq.irqType, irq.irqNumber, irq.deviceName);
        } else {
            displayText = QStringLiteral("%1 %2").arg(irq.irqNumber, irq.deviceName);
        }
        auto *node = new Node({displayText, s::empty()}, irqItem);
        node->setIcon(s::categoryIcons::irq());
        irqItem->appendChild(node);
    }

    hostnameItem->appendChild(irqItem);
}

void ResourcesByConnectionModel::addMemory() {
    auto ranges = getSystemMemoryRanges();
    if (ranges.isEmpty()) {
        return;
    }

    memoryItem = new Node({tr("Memory"), s::empty()}, hostnameItem);
    memoryItem->setIcon(s::categoryIcons::memory());

    QStack<QPair<int, Node *>> nodeStack;
    nodeStack.push({-1, memoryItem});

    for (const auto &range : ranges) {
        buildHierarchicalResource(memoryItem,
                                  s::categoryIcons::memory(),
                                  range.indentLevel,
                                  range.rangeStart,
                                  range.rangeEnd,
                                  range.name,
                                  nodeStack);
    }

    hostnameItem->appendChild(memoryItem);
}

int ResourcesByConnectionModel::columnCount([[maybe_unused]] const QModelIndex &parent) const {
    return 1;
}

QVariant
ResourcesByConnectionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Resource");
        case 1:
            return tr("Details");
        default:
            return {};
        }
    }
    return {};
}
