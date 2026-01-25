#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QVBoxLayout>

#include "customizedialog.h"

CustomizeDialog::CustomizeDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Customize View"));
    setMinimumWidth(300);

    auto *layout = new QVBoxLayout(this);

    // View options group
    auto *viewGroup = new QGroupBox(tr("View Options"), this);
    auto *viewLayout = new QVBoxLayout(viewGroup);

    showIconsCheckBox = new QCheckBox(tr("Show device icons"), viewGroup);
    showIconsCheckBox->setChecked(true);
    viewLayout->addWidget(showIconsCheckBox);

    expandAllCheckBox = new QCheckBox(tr("Expand all categories on load"), viewGroup);
    expandAllCheckBox->setChecked(false);
    viewLayout->addWidget(expandAllCheckBox);

    showDriverColumnCheckBox =
        new QCheckBox(tr("Show driver column (Devices by type view only)"), viewGroup);
    showDriverColumnCheckBox->setChecked(false);
    viewLayout->addWidget(showDriverColumnCheckBox);

    layout->addWidget(viewGroup);

    layout->addStretch();

    // Buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

bool CustomizeDialog::showDeviceIcons() const {
    return showIconsCheckBox->isChecked();
}

void CustomizeDialog::setShowDeviceIcons(bool show) {
    showIconsCheckBox->setChecked(show);
}

bool CustomizeDialog::expandAllOnLoad() const {
    return expandAllCheckBox->isChecked();
}

void CustomizeDialog::setExpandAllOnLoad(bool expand) {
    expandAllCheckBox->setChecked(expand);
}

bool CustomizeDialog::showDriverColumn() const {
    return showDriverColumnCheckBox->isChecked();
}

void CustomizeDialog::setShowDriverColumn(bool show) {
    showDriverColumnCheckBox->setChecked(show);
}
