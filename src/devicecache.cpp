#include "devicecache.h"
#include "devicemonitor.h"
#include "systeminfo.h"
#include "viewsettings.h"

#include <QFile>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

DeviceCache &DeviceCache::instance() {
    static DeviceCache cache;
    return cache;
}

const QString &DeviceCache::hostname() {
    // In viewer mode, return the imported hostname
    if (instance().viewerMode_ && !instance().importedHostname_.isEmpty()) {
        return instance().importedHostname_;
    }
    static QString cachedHostname = QHostInfo::localHostName();
    return cachedHostname;
}

DeviceCache::DeviceCache() : QObject(nullptr) {
    enumerate();
    startMonitoring();
}

DeviceCache::~DeviceCache() {
    if (monitor_) {
        auto *deviceMonitor = qobject_cast<DeviceMonitor *>(monitor_);
        if (deviceMonitor) {
            deviceMonitor->stop();
        }
        delete monitor_;
        monitor_ = nullptr;
    }
}

void DeviceCache::enumerate() {
    devices_.clear();
    syspathIndex_.clear();

    devices_ = enumerateAllDevices();

    // Build the syspath index
    for (int i = 0; i < devices_.size(); ++i) {
        const QString &syspath = devices_.at(i).syspath();
        if (!syspath.isEmpty()) {
            syspathIndex_.insert(syspath, i);
        }
    }
}

QList<DeviceInfo> DeviceCache::allDevices() const {
    QMutexLocker locker(&mutex_);
    return devices_; // Returns a copy for thread safety
}

const DeviceInfo *DeviceCache::deviceBySyspath(const QString &syspath) const {
    QMutexLocker locker(&mutex_);
    auto it = syspathIndex_.find(syspath);
    if (it != syspathIndex_.end()) {
        return &devices_.at(it.value());
    }
    return nullptr;
}

void DeviceCache::refresh() {
    QMutexLocker locker(&mutex_);
    enumerate();
}

bool DeviceCache::showHiddenDevices() const {
    return ViewSettings::instance().showHiddenDevices();
}

void DeviceCache::setShowHiddenDevices(bool show) {
    ViewSettings::instance().setShowHiddenDevices(show);
    ViewSettings::instance().save();
}

void DeviceCache::startMonitoring() {
    monitor_ = createDeviceMonitor(this);
    if (monitor_) {
        auto *deviceMonitor = qobject_cast<DeviceMonitor *>(monitor_);
        if (deviceMonitor) {
            connect(
                deviceMonitor, &DeviceMonitor::deviceChanged, this, &DeviceCache::onDeviceChanged);
            deviceMonitor->start();
        }
    }
}

void DeviceCache::onDeviceChanged() {
    // Don't refresh in viewer mode
    if (viewerMode_) {
        return;
    }
    refresh();
    Q_EMIT devicesChanged();
}

bool DeviceCache::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();

    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();

    // Verify this is a valid export file by checking for required fields
    if (!root.contains(QStringLiteral("formatVersion")) ||
        !root.contains(QStringLiteral("devices"))) {
        return false;
    }

    QMutexLocker locker(&mutex_);

    // Stop monitoring while in viewer mode
    if (monitor_) {
        auto *deviceMonitor = qobject_cast<DeviceMonitor *>(monitor_);
        if (deviceMonitor) {
            deviceMonitor->stop();
        }
    }

    // Clear existing data
    devices_.clear();
    syspathIndex_.clear();

    // Load metadata
    viewerMode_ = true;
    filePath_ = filePath;
    systemInfo_ = root[QStringLiteral("system")].toObject();
    importedHostname_ = systemInfo_[QStringLiteral("hostname")].toString();
    exportDate_ = root[QStringLiteral("exportDate")].toString();
    sourceAppName_ = root[QStringLiteral("applicationName")].toString();
    sourceAppVersion_ = root[QStringLiteral("applicationVersion")].toString();
    systemResources_ = root[QStringLiteral("systemResources")].toObject();

    // Load devices
    QJsonArray devicesArray = root[QStringLiteral("devices")].toArray();
    devices_.reserve(devicesArray.size());

    for (const QJsonValue val : devicesArray) {
        if (!val.isObject()) {
            continue;
        }
        devices_.emplaceBack(val.toObject());
        const QString &syspath = devices_.last().syspath();
        if (!syspath.isEmpty()) {
            syspathIndex_.insert(syspath, static_cast<int>(devices_.size() - 1));
        }
    }

    locker.unlock();
    Q_EMIT devicesChanged();
    return true;
}

bool DeviceCache::isViewerMode() const {
    return viewerMode_;
}

void DeviceCache::reloadLiveData() {
    QMutexLocker locker(&mutex_);

    // Clear viewer mode state
    viewerMode_ = false;
    filePath_.clear();
    importedHostname_.clear();
    exportDate_.clear();
    sourceAppName_.clear();
    sourceAppVersion_.clear();
    systemInfo_ = QJsonObject();
    systemResources_ = QJsonObject();

    // Re-enumerate live devices
    enumerate();

    // Restart monitoring
    if (monitor_) {
        auto *deviceMonitor = qobject_cast<DeviceMonitor *>(monitor_);
        if (deviceMonitor) {
            deviceMonitor->start();
        }
    }

    locker.unlock();
    Q_EMIT devicesChanged();
}

const QString &DeviceCache::currentFilePath() const {
    return filePath_;
}

const QString &DeviceCache::exportDate() const {
    return exportDate_;
}

const QString &DeviceCache::sourceApplicationName() const {
    return sourceAppName_;
}

const QString &DeviceCache::sourceApplicationVersion() const {
    return sourceAppVersion_;
}

const QJsonObject &DeviceCache::systemInfo() const {
    return systemInfo_;
}

const QJsonObject &DeviceCache::systemResources() const {
    return systemResources_;
}
