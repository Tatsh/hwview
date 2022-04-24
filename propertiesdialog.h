#include <QtWidgets/QDialog>

#include "ui_propertiesdialog.h"

class PropertiesDialog : public QDialog, private Ui::PropertiesDialog {
    Q_OBJECT

public:
    PropertiesDialog();
    void setDeviceName(QString &);
    void setDriverName(QString &);
};
