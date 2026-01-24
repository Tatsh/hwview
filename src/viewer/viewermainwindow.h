#pragma once

#include <QMainWindow>
#include <QSet>

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QActionGroup;
class QTreeView;
QT_END_NAMESPACE

/**
 * @brief The main window for Device Manager Viewer.
 *
 * This class provides a read-only view of exported device data. It supports
 * multiple view modes but does not have export, refresh, or device actions.
 */
class ViewerMainWindow : public QMainWindow {
    Q_OBJECT

public:
    ViewerMainWindow();
    ~ViewerMainWindow() override = default;

    /**
     * @brief Opens a file dialog to select an export file to view.
     * @returns @c true if a file was loaded successfully.
     */
    bool openFile();

    /**
     * @brief Loads the specified export file.
     * @param filePath Path to the .dmexport file.
     * @returns @c true if loaded successfully.
     */
    bool loadFile(const QString &filePath);

private Q_SLOTS:
    void about();
    void switchToDevicesByType();
    void switchToModel(QAbstractItemModel *model, int depth = 0);
    void applyViewSettings();
    void openPropertiesForIndex(const QModelIndex &index);
    void openNewFile();
    void toggleShowHiddenDevices();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();
    void setupMenus();
    void updateWindowTitle();
    QSet<QString> saveExpandedState() const;
    void restoreExpandedState(const QSet<QString> &expandedPaths);
    void collectExpandedPaths(const QModelIndex &parent,
                              const QString &parentPath,
                              QSet<QString> &expandedPaths) const;
    void expandMatchingPaths(const QModelIndex &parent,
                             const QString &parentPath,
                             const QSet<QString> &expandedPaths);

    QTreeView *treeView_;
    QActionGroup *actionGroupView_;
    QAction *currentViewAction_ = nullptr;

    // View actions
    QAction *actionDevicesByType_;
    QAction *actionDevicesByConnection_;
    QAction *actionDevicesByDriver_;
    QAction *actionDriversByType_;
    QAction *actionDriversByDevice_;
    QAction *actionResourcesByType_;
    QAction *actionResourcesByConnection_;
    QAction *actionShowHiddenDevices_;
};
