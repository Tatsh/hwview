#include <QtWidgets/QDialog>

#include "ui_propertiesdialog.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class PropertiesDialog : public QDialog, private Ui::PropertiesDialog {
    Q_OBJECT

public:
    PropertiesDialog();
    void setDeviceName(QString &);
};
