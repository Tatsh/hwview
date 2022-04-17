#include <QtWidgets/QMainWindow>

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
class QActionGroup;
QT_END_NAMESPACE

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT

public:
    MainWindow();

private Q_SLOTS:
    void about();

private:
    QActionGroup *actionGroupView;
};
