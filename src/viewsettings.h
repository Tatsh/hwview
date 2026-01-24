#pragma once

#include <QObject>
#include <QString>

/**
 * @brief Singleton class that holds view customization settings.
 *
 * This class manages persistent settings for the device tree view,
 * including icon display, expansion behavior, column visibility,
 * and hidden device visibility.
 *
 * Settings are automatically loaded on first access and can be
 * saved to persistent storage using save().
 */
class ViewSettings : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Returns the singleton instance.
     * @returns Reference to the global ViewSettings instance.
     */
    static ViewSettings &instance();

    /**
     * @brief Returns whether device icons should be displayed.
     * @returns @c true if icons should be shown, @c false otherwise.
     */
    bool showDeviceIcons() const;

    /**
     * @brief Sets whether device icons should be displayed.
     * @param show @c true to show icons, @c false to hide them.
     */
    void setShowDeviceIcons(bool show);

    /**
     * @brief Returns whether the tree should expand all nodes on load.
     * @returns @c true if all nodes should expand, @c false otherwise.
     */
    bool expandAllOnLoad() const;

    /**
     * @brief Sets whether the tree should expand all nodes on load.
     * @param expand @c true to expand all nodes, @c false for default.
     */
    void setExpandAllOnLoad(bool expand);

    /**
     * @brief Returns whether the driver column should be shown.
     * @returns @c true if the driver column is visible, @c false otherwise.
     */
    bool showDriverColumn() const;

    /**
     * @brief Sets whether the driver column should be shown.
     * @param show @c true to show the column, @c false to hide it.
     */
    void setShowDriverColumn(bool show);

    /**
     * @brief Returns whether hidden devices should be displayed.
     * @returns @c true if hidden devices are shown, @c false otherwise.
     */
    bool showHiddenDevices() const;

    /**
     * @brief Sets whether hidden devices should be displayed.
     * @param show @c true to show hidden devices, @c false to hide them.
     */
    void setShowHiddenDevices(bool show);

    /**
     * @brief Returns the name of the last used view.
     * @returns The view name (e.g., "DevicesByType").
     */
    QString lastView() const;

    /**
     * @brief Sets the last used view name.
     * @param view The view name to save.
     */
    void setLastView(const QString &view);

    /**
     * @brief Saves all settings to persistent storage.
     */
    void save();

    /**
     * @brief Loads all settings from persistent storage.
     */
    void load();

Q_SIGNALS:
    /**
     * @brief Emitted when any setting changes.
     */
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
