#include <QRegularExpression>

#include "const_strings.h"
#include "devicecache.h"
#include "models/resbytypemodel.h"
#include "procutils.h"
#include "viewsettings.h"

namespace s = strings;

#ifdef Q_OS_LINUX
using procutils::readProcFile;
#endif

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
#ifdef Q_OS_LINUX
    addDma();
    addIoPorts();
    addIrq();
    addMemory();
#endif
}

#ifdef Q_OS_LINUX
void ResourcesByTypeModel::addDma() {
    dmaItem = new Node({tr("Direct memory access (DMA)"), s::empty()}, hostnameItem);
    dmaItem->setIcon(s::categoryIcons::other());

    static const QRegularExpression dmaRe(QStringLiteral("^(\\d+):\\s*(.*)$"));
    for (const auto &line : readProcFile(QStringLiteral("/proc/dma"))) {
        if (line.trimmed().isEmpty()) {
            continue;
        }
        // Format: "4: cascade"
        auto match = dmaRe.match(line.trimmed());
        if (match.hasMatch()) {
            auto channel = match.captured(1);
            auto name = match.captured(2);
            auto displayText = QStringLiteral("[%1] %2").arg(channel, name);
            auto *node = new Node({displayText, s::empty()}, dmaItem);
            node->setIcon(s::categoryIcons::other());
            dmaItem->appendChild(node);
        }
    }

    if (dmaItem->childCount() > 0) {
        hostnameItem->appendChild(dmaItem);
    } else {
        delete dmaItem;
        dmaItem = nullptr;
    }
}

void ResourcesByTypeModel::addIoPorts() {
    ioItem = new Node({tr("Input/output (IO)"), s::empty()}, hostnameItem);
    ioItem->setIcon(s::categoryIcons::other());

    static const QRegularExpression ioRe(
        QStringLiteral("^\\s*([0-9a-fA-F]+)-([0-9a-fA-F]+)\\s*:\\s*(.*)$"));
    for (const auto &line : readProcFile(QStringLiteral("/proc/ioports"))) {
        if (line.trimmed().isEmpty()) {
            continue;
        }
        // Format: "  0000-0cf7 : PCI Bus 0000:00"
        // Indentation indicates hierarchy, but we'll flatten for simplicity
        auto match = ioRe.match(line);
        if (match.hasMatch()) {
            auto rangeStart = match.captured(1).toUpper();
            auto rangeEnd = match.captured(2).toUpper();
            auto name = match.captured(3);
            // Skip empty or generic entries
            if (name.isEmpty() || name.startsWith(QStringLiteral("PCI Bus"))) {
                continue;
            }
            auto displayText = QStringLiteral("[%1 - %2] %3").arg(rangeStart, rangeEnd, name);
            auto *node = new Node({displayText, s::empty()}, ioItem);
            node->setIcon(s::categoryIcons::other());
            ioItem->appendChild(node);
        }
    }

    if (ioItem->childCount() > 0) {
        hostnameItem->appendChild(ioItem);
    } else {
        delete ioItem;
        ioItem = nullptr;
    }
}

void ResourcesByTypeModel::addIrq() {
    irqItem = new Node({tr("Interrupt request (IRQ)"), s::empty()}, hostnameItem);
    irqItem->setIcon(s::categoryIcons::other());

    static const QRegularExpression whitespaceRe(QStringLiteral("\\s+"));

    auto lines = readProcFile(QStringLiteral("/proc/interrupts"));
    // Skip header line
    if (!lines.isEmpty()) {
        lines.removeFirst();
    }
    for (const auto &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }
        // Format: "  0:         23   IO-APIC   2-edge      timer"
        // or:     "NMI:          0          0   Non-maskable interrupts"
        // First column is IRQ number or name, last column(s) are device name
        auto parts = line.split(whitespaceRe, Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }
        auto irqNum = parts[0];
        if (irqNum.endsWith(QLatin1Char(':'))) {
            irqNum.chop(1);
        }
        // The device name is typically the last part(s)
        // Find the interrupt type (like IO-APIC, PCI-MSI, etc.) and device name
        QString deviceName;
        QString irqType;

        // Look for common IRQ type indicators
        for (auto i = 1; i < parts.size(); ++i) {
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
                // This is likely the device name
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

        QString displayText;
        if (!irqType.isEmpty()) {
            displayText = QStringLiteral("(%1) %2 %3").arg(irqType, irqNum, deviceName);
        } else {
            displayText = QStringLiteral("%1 %2").arg(irqNum, deviceName);
        }
        auto *node = new Node({displayText, s::empty()}, irqItem);
        node->setIcon(s::categoryIcons::other());
        irqItem->appendChild(node);
    }

    if (irqItem->childCount() > 0) {
        hostnameItem->appendChild(irqItem);
    } else {
        delete irqItem;
        irqItem = nullptr;
    }
}

void ResourcesByTypeModel::addMemory() {
    memoryItem = new Node({tr("Memory"), s::empty()}, hostnameItem);
    memoryItem->setIcon(s::categoryIcons::other());

    static const QRegularExpression memRe(
        QStringLiteral("^\\s*([0-9a-fA-F]+)-([0-9a-fA-F]+)\\s*:\\s*(.*)$"));
    for (const auto &line : readProcFile(QStringLiteral("/proc/iomem"))) {
        if (line.trimmed().isEmpty()) {
            continue;
        }
        // Format: "  00000000-00000fff : Reserved"
        auto match = memRe.match(line);
        if (match.hasMatch()) {
            auto rangeStart = match.captured(1).toUpper();
            auto rangeEnd = match.captured(2).toUpper();
            auto name = match.captured(3);
            // Skip generic/system entries
            if (name.isEmpty() || name == QStringLiteral("Reserved") ||
                name == QStringLiteral("System RAM") || name == QStringLiteral("System ROM") ||
                name.startsWith(QStringLiteral("PCI Bus")) ||
                name.startsWith(QStringLiteral("PCI MMCONFIG")) ||
                name.startsWith(QStringLiteral("ACPI")) ||
                name.startsWith(QStringLiteral("0000:"))) {
                continue;
            }
            auto displayText = QStringLiteral("[%1 - %2] %3").arg(rangeStart, rangeEnd, name);
            auto *node = new Node({displayText, s::empty()}, memoryItem);
            node->setIcon(s::categoryIcons::other());
            memoryItem->appendChild(node);
        }
    }

    if (memoryItem->childCount() > 0) {
        hostnameItem->appendChild(memoryItem);
    } else {
        delete memoryItem;
        memoryItem = nullptr;
    }
}
#endif

int ResourcesByTypeModel::columnCount([[maybe_unused]] const QModelIndex &parent) const {
    return ViewSettings::instance().showDriverColumn() ? 2 : 1;
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
