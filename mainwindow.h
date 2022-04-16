#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE
class QAction;
class QTreeView;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private Q_SLOTS:
    void about();

private:
    void createActions();
    void setupTreeView();

    QAction *updateDriverAction;
    QAction *updateDriverSeparatorAction;
    QTreeView *treeView;
};
