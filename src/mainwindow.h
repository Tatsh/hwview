#pragma once

#ifdef DEVMGMT_USE_KDE
#include <KXmlGuiWindow>
#else
#include <QMainWindow>
#endif

#include <QFutureWatcher>
#include <QSet>

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QActionGroup;
class QProgressDialog;
QT_END_NAMESPACE

/**
 * @brief The main application window for Device Manager.
 *
 * This class provides the main user interface for viewing and managing devices. It supports
 * multiple view modes (Devices by Type, Devices by Connection, etc.) and automatically refreshes
 * when devices are added or removed.
 *
 * On KDE, this class inherits from @c KXmlGuiWindow for proper KDE integration. On other platforms,
 * it inherits from @c QMainWindow.
 *
 * Key features:
 * - Multiple device view modes
 * - Automatic refresh on device changes (Linux)
 * - Tree state preservation during refresh
 * - Device properties dialog
 * - Hardware scan functionality
 */
#ifdef DEVMGMT_USE_KDE
class MainWindow : public KXmlGuiWindow, private Ui::MainWindow {
#else
class MainWindow : public QMainWindow, private Ui::MainWindow {
#endif
    Q_OBJECT

public:
    /**
     * @brief Constructs the main window.
     *
     * Initializes the UI, sets up menus and actions, restores the last view, and connects to the
     * device monitor for automatic updates.
     */
    MainWindow();
    ~MainWindow() override;

private Q_SLOTS:
    void about();
    void switchToDevicesByType();
    void switchToModel(QAbstractItemModel *model, int depth = 0);
    void toggleShowHiddenDevices(bool checked);
    void refreshCurrentView();
    void scanForHardwareChanges();
    void onScanComplete();
    void showCustomizeDialog();
    void applyViewSettings();
    void openPropertiesForIndex(const QModelIndex &index);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupMenus();
    void restoreLastView();
    void connectDeviceMonitor();
    QSet<QString> saveExpandedState() const;
    void restoreExpandedState(const QSet<QString> &expandedPaths);
    void collectExpandedPaths(const QModelIndex &parent,
                              const QString &parentPath,
                              QSet<QString> &expandedPaths) const;
    void expandMatchingPaths(const QModelIndex &parent,
                             const QString &parentPath,
                             const QSet<QString> &expandedPaths);

#ifdef DEVMGMT_USE_KDE
    void setupActions();
    void postSetupMenus();
#endif

    QActionGroup *actionGroupView;
    QAction *currentViewAction = nullptr;
    QProgressDialog *scanProgressDialog = nullptr;
    QFutureWatcher<void> *scanWatcher_ = nullptr;
};
