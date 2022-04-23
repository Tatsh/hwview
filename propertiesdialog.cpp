#include "propertiesdialog.h"

PropertiesDialog::PropertiesDialog() {
    setupUi(this);
}

void PropertiesDialog::setDeviceName(QString &name) {
    labelDeviceName->setText(name);
}
