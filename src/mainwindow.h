#pragma once

#ifdef DEVMGMT_USE_KDE
#include <KXmlGuiWindow>
#else
#include <QMainWindow>
#endif

#include <QFutureWatcher>

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QActionGroup;
class QProgressDialog;
QT_END_NAMESPACE

#ifdef DEVMGMT_USE_KDE
class MainWindow : public KXmlGuiWindow, private Ui::MainWindow {
#else
class MainWindow : public QMainWindow, private Ui::MainWindow {
#endif
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;

private Q_SLOTS:
    void about();
    void switchToDevicesByType();
    void switchToModel(QAbstractItemModel *, int depth = 0);
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
#ifdef DEVMGMT_USE_KDE
    void setupActions();
    void postSetupMenus();
#endif

    QActionGroup *actionGroupView;
    QAction *currentViewAction = nullptr;
    QProgressDialog *scanProgressDialog = nullptr;
    QFutureWatcher<void> *scanWatcher_ = nullptr;
};
