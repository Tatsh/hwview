#include <QtWidgets/QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();

private Q_SLOTS:
    void about();

private:
    void createActions();
};
