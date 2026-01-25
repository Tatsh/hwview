#include "const_strings.h"
#include "devicecache.h"
#include "models/resbytypemodel.h"
#include "systeminfo.h"
#include "viewsettings.h"

namespace s = strings;

ResourcesByTypeModel::ResourcesByTypeModel(QObject *parent)
    : BaseTreeModel(parent), hostnameItem(nullptr), dmaItem(nullptr), ioItem(nullptr),
      irqItem(nullptr), memoryItem(nullptr) {
    auto *root = new Node({s::empty(), s::empty()});
    setRootItem(root);
    hostnameItem = new Node({DeviceCache::hostname(), s::empty()}, root);
    hostnameItem->setIcon(s::categoryIcons::computer());
    root->appendChild(hostnameItem);
    buildTree();
}

bool ResourcesByTypeModel::shouldShowIcons() const {
    return ViewSettings::instance().showDeviceIcons();
}

void ResourcesByTypeModel::buildTree() {
    addDma();
    addIoPorts();
    addIrq();
    addMemory();
}

void ResourcesByTypeModel::addDma() {
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

void ResourcesByTypeModel::addIoPorts() {
    auto ports = getSystemIoPorts();
    if (ports.isEmpty()) {
        return;
    }

    ioItem = new Node({tr("Input/output (IO)"), s::empty()}, hostnameItem);
    ioItem->setIcon(s::categoryIcons::ioPorts());

    for (const auto &port : ports) {
        // Skip generic entries for the flat view
        if (port.name.startsWith(QStringLiteral("PCI Bus"))) {
            continue;
        }
        auto displayText =
            QStringLiteral("[%1 - %2] %3").arg(port.rangeStart, port.rangeEnd, port.name);
        auto *node = new Node({displayText, s::empty()}, ioItem);
        node->setIcon(s::categoryIcons::ioPorts());
        ioItem->appendChild(node);
    }

    if (ioItem->childCount() > 0) {
        hostnameItem->appendChild(ioItem);
    } else {
        delete ioItem;
        ioItem = nullptr;
    }
}

void ResourcesByTypeModel::addIrq() {
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

void ResourcesByTypeModel::addMemory() {
    auto ranges = getSystemMemoryRanges();
    if (ranges.isEmpty()) {
        return;
    }

    memoryItem = new Node({tr("Memory"), s::empty()}, hostnameItem);
    memoryItem->setIcon(s::categoryIcons::memory());

    for (const auto &range : ranges) {
        // Skip generic/system entries for the flat view
        if (range.name.isEmpty() || range.name == QStringLiteral("Reserved") ||
            range.name == QStringLiteral("System RAM") ||
            range.name == QStringLiteral("System ROM") ||
            range.name.startsWith(QStringLiteral("PCI Bus")) ||
            range.name.startsWith(QStringLiteral("PCI MMCONFIG")) ||
            range.name.startsWith(QStringLiteral("ACPI")) ||
            range.name.startsWith(QStringLiteral("0000:"))) {
            continue;
        }
        auto displayText =
            QStringLiteral("[%1 - %2] %3").arg(range.rangeStart, range.rangeEnd, range.name);
        auto *node = new Node({displayText, s::empty()}, memoryItem);
        node->setIcon(s::categoryIcons::memory());
        memoryItem->appendChild(node);
    }

    if (memoryItem->childCount() > 0) {
        hostnameItem->appendChild(memoryItem);
    } else {
        delete memoryItem;
        memoryItem = nullptr;
    }
}

int ResourcesByTypeModel::columnCount([[maybe_unused]] const QModelIndex &parent) const {
    return 1;
}

QVariant
ResourcesByTypeModel::headerData(int section, Qt::Orientation orientation, int role) const {
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
