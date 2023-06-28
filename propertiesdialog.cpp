#include "propertiesdialog.h"

PropertiesDialog::PropertiesDialog() {
    setupUi(this);
}

void PropertiesDialog::setDeviceName(QString &name) {
    labelDeviceNameGeneral->setText(name);
    labelDeviceNameDetails->setText(name);
    labelDeviceNameDriver->setText(name);
    labelDeviceNameEvents->setText(name);
    listWidgetDetailsPropertyValue->insertItem(0, name);
    deviceName_ = name;
}

void PropertiesDialog::setDriverName(QString &name) {
    driverName_ = name;
}
