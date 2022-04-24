#include "propertiesdialog.h"

PropertiesDialog::PropertiesDialog() {
    setupUi(this);
}

void PropertiesDialog::setDeviceName(QString &name) {
    labelDeviceNameGeneral->setText(name);
    labelDeviceNameDetails->setText(name);
    labelDeviceNameDriver->setText(name);
    labelDeviceNameEvents->setText(name);
}
