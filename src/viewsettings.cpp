#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>

#include "viewsettings.h"

namespace {
QSettings createSettings() {
    // Use platform-appropriate config location
    auto configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return QSettings(configPath + QStringLiteral("/hwviewrc"), QSettings::IniFormat);
}
} // namespace

ViewSettings &ViewSettings::instance() {
    static ViewSettings settings;
    return settings;
}

ViewSettings::ViewSettings() : QObject(nullptr) {
    load();
}

bool ViewSettings::showDeviceIcons() const {
    return showDeviceIcons_;
}

void ViewSettings::setShowDeviceIcons(bool show) {
    if (showDeviceIcons_ != show) {
        showDeviceIcons_ = show;
        Q_EMIT settingsChanged();
    }
}

bool ViewSettings::expandAllOnLoad() const {
    return expandAllOnLoad_;
}

void ViewSettings::setExpandAllOnLoad(bool expand) {
    if (expandAllOnLoad_ != expand) {
        expandAllOnLoad_ = expand;
        Q_EMIT settingsChanged();
    }
}

bool ViewSettings::showDriverColumn() const {
    return showDriverColumn_;
}

void ViewSettings::setShowDriverColumn(bool show) {
    if (showDriverColumn_ != show) {
        showDriverColumn_ = show;
        Q_EMIT settingsChanged();
    }
}

bool ViewSettings::showHiddenDevices() const {
    return showHiddenDevices_;
}

void ViewSettings::setShowHiddenDevices(bool show) {
    if (showHiddenDevices_ != show) {
        showHiddenDevices_ = show;
        Q_EMIT settingsChanged();
    }
}

QString ViewSettings::lastView() const {
    return lastView_;
}

void ViewSettings::setLastView(const QString &view) {
    if (lastView_ != view) {
        lastView_ = view;
        // Save immediately when view changes
        save();
    }
}

void ViewSettings::save() {
    auto settings = createSettings();
    settings.beginGroup(QStringLiteral("View"));
    settings.setValue(QStringLiteral("showDeviceIcons"), showDeviceIcons_);
    settings.setValue(QStringLiteral("expandAllOnLoad"), expandAllOnLoad_);
    settings.setValue(QStringLiteral("showDriverColumn"), showDriverColumn_);
    settings.setValue(QStringLiteral("showHiddenDevices"), showHiddenDevices_);
    settings.setValue(QStringLiteral("lastView"), lastView_);
    settings.endGroup();
}

void ViewSettings::load() {
    auto settings = createSettings();
    settings.beginGroup(QStringLiteral("View"));
    showDeviceIcons_ = settings.value(QStringLiteral("showDeviceIcons"), true).toBool();
    expandAllOnLoad_ = settings.value(QStringLiteral("expandAllOnLoad"), false).toBool();
    showDriverColumn_ = settings.value(QStringLiteral("showDriverColumn"), false).toBool();
    showHiddenDevices_ = settings.value(QStringLiteral("showHiddenDevices"), false).toBool();
    lastView_ =
        settings.value(QStringLiteral("lastView"), QStringLiteral("DevicesByType")).toString();
    settings.endGroup();
}
