#pragma once

#include <QObject>
#include <QString>

// Singleton to hold view customization settings
class ViewSettings : public QObject {
    Q_OBJECT

public:
    static ViewSettings &instance();

    bool showDeviceIcons() const;
    void setShowDeviceIcons(bool show);

    bool expandAllOnLoad() const;
    void setExpandAllOnLoad(bool expand);

    bool showDriverColumn() const;
    void setShowDriverColumn(bool show);

    bool showHiddenDevices() const;
    void setShowHiddenDevices(bool show);

    QString lastView() const;
    void setLastView(const QString &view);

    void save();
    void load();

Q_SIGNALS:
    void settingsChanged();

private:
    ViewSettings();
    ~ViewSettings() override = default;
    ViewSettings(const ViewSettings &) = delete;
    ViewSettings &operator=(const ViewSettings &) = delete;

    bool showDeviceIcons_ = true;
    bool expandAllOnLoad_ = false;
    bool showDriverColumn_ = false;
    bool showHiddenDevices_ = false;
    QString lastView_;
};
