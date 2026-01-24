#include "driverdetailsdialog.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpacerItem>
#include <QVBoxLayout>

#ifdef Q_OS_LINUX
#include <sys/utsname.h>
#endif

DriverDetailsDialog::DriverDetailsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(tr("Driver File Details"));
    setMinimumSize(450, 350);

    auto *mainLayout = new QVBoxLayout(this);

    // Top section with icon and driver name
    auto *topLayout = new QHBoxLayout();
    labelIcon_ = new QLabel(this);
    labelIcon_->setFixedSize(32, 32);
    topLayout->addWidget(labelIcon_);

    topLayout->addSpacing(10);

    labelDeviceName_ = new QLabel(this);
    labelDeviceName_->setWordWrap(true);
    topLayout->addWidget(labelDeviceName_, 1);

    mainLayout->addLayout(topLayout);
    mainLayout->addSpacing(10);

    // Driver files section
    auto *filesLabel = new QLabel(tr("Driver files:"), this);
    mainLayout->addWidget(filesLabel);

    listDriverFiles_ = new QListWidget(this);
    listDriverFiles_->setMinimumHeight(100);
    connect(listDriverFiles_,
            &QListWidget::currentRowChanged,
            this,
            &DriverDetailsDialog::onFileSelectionChanged);
    mainLayout->addWidget(listDriverFiles_);

    mainLayout->addSpacing(10);

    // File details grid
    auto *detailsLayout = new QGridLayout();
    detailsLayout->setColumnStretch(1, 1);

    int row = 0;

    labelProvider_ = new QLabel(tr("Provider:"), this);
    labelProviderValue_ = new QLabel(this);
    labelProviderValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelProvider_, row, 0);
    detailsLayout->addWidget(labelProviderValue_, row, 1);
    row++;

    labelFileVersion_ = new QLabel(tr("File version:"), this);
    labelFileVersionValue_ = new QLabel(this);
    labelFileVersionValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelFileVersion_, row, 0);
    detailsLayout->addWidget(labelFileVersionValue_, row, 1);
    row++;

    labelCopyright_ = new QLabel(tr("Copyright:"), this);
    labelCopyrightValue_ = new QLabel(this);
    labelCopyrightValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    labelCopyrightValue_->setWordWrap(true);
    detailsLayout->addWidget(labelCopyright_, row, 0);
    detailsLayout->addWidget(labelCopyrightValue_, row, 1);
    row++;

    labelDigitalSigner_ = new QLabel(tr("Digital signer:"), this);
    labelDigitalSignerValue_ = new QLabel(this);
    labelDigitalSignerValue_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsLayout->addWidget(labelDigitalSigner_, row, 0);
    detailsLayout->addWidget(labelDigitalSignerValue_, row, 1);

    mainLayout->addLayout(detailsLayout);

    // Spacer
    mainLayout->addStretch();

    // OK button
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    auto *okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okButton);
    mainLayout->addLayout(buttonLayout);
}

void DriverDetailsDialog::setDriverName(const QString &driverName) {
    driverName_ = driverName;
    labelDeviceName_->setText(tr("Driver: %1").arg(driverName));
    populateDriverFiles();
}

void DriverDetailsDialog::setCategoryIcon(const QIcon &icon) {
    categoryIcon_ = icon;
    labelIcon_->setPixmap(icon.pixmap(32, 32));
}

#ifdef Q_OS_LINUX
void DriverDetailsDialog::populateDriverFiles() {
    listDriverFiles_->clear();

    if (driverName_.isEmpty()) {
        return;
    }

    // Look for the module file
    QStringList modulePaths;

    // Use modinfo to find the actual module file
    QProcess modinfo;
    modinfo.start(QStringLiteral("modinfo"),
                  {QStringLiteral("-F"), QStringLiteral("filename"), driverName_});
    if (modinfo.waitForFinished(3000)) {
        QString filename = QString::fromUtf8(modinfo.readAllStandardOutput()).trimmed();
        if (!filename.isEmpty() && filename != QStringLiteral("(builtin)")) {
            modulePaths << filename;
        } else if (filename == QStringLiteral("(builtin)")) {
            // Built-in module
            listDriverFiles_->addItem(tr("(Built-in kernel module)"));
        }
    }

    // Also check for related modules (dependencies)
    modinfo.start(QStringLiteral("modinfo"),
                  {QStringLiteral("-F"), QStringLiteral("depends"), driverName_});
    if (modinfo.waitForFinished(3000)) {
        QString depends = QString::fromUtf8(modinfo.readAllStandardOutput()).trimmed();
        if (!depends.isEmpty()) {
            QStringList depList = depends.split(QStringLiteral(","), Qt::SkipEmptyParts);
            for (const QString &dep : depList) {
                QProcess depInfo;
                depInfo.start(QStringLiteral("modinfo"),
                              {QStringLiteral("-F"), QStringLiteral("filename"), dep.trimmed()});
                if (depInfo.waitForFinished(3000)) {
                    QString depFile = QString::fromUtf8(depInfo.readAllStandardOutput()).trimmed();
                    if (!depFile.isEmpty() && depFile != QStringLiteral("(builtin)") &&
                        !modulePaths.contains(depFile)) {
                        modulePaths << depFile;
                    }
                }
            }
        }
    }

    // Add files to list
    for (const QString &path : modulePaths) {
        auto *item = new QListWidgetItem(path);
        item->setData(Qt::UserRole, path);
        listDriverFiles_->addItem(item);
    }

    // Select first item
    if (listDriverFiles_->count() > 0) {
        listDriverFiles_->setCurrentRow(0);
    }
}
#else
void DriverDetailsDialog::populateDriverFiles() {
    listDriverFiles_->clear();
    listDriverFiles_->addItem(tr("(Driver details not available on this platform)"));
}
#endif

#ifdef Q_OS_LINUX
void DriverDetailsDialog::onFileSelectionChanged() {
    QListWidgetItem *item = listDriverFiles_->currentItem();
    if (!item) {
        labelProviderValue_->clear();
        labelFileVersionValue_->clear();
        labelCopyrightValue_->clear();
        labelDigitalSignerValue_->clear();
        return;
    }

    QString modulePath = item->data(Qt::UserRole).toString();
    if (modulePath.isEmpty()) {
        // Built-in module
        labelProviderValue_->setText(QStringLiteral("Linux Foundation"));
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            labelFileVersionValue_->setText(QString::fromLocal8Bit(buffer.release));
        }
        labelCopyrightValue_->setText(tr("GPL-compatible"));
        labelDigitalSignerValue_->setText(QStringLiteral("Linux Foundation"));
        return;
    }

    updateFileDetails(modulePath);
}

void DriverDetailsDialog::updateFileDetails(const QString &modulePath) {
    // Extract module name from path
    QString moduleName = modulePath;
    auto lastSlash = moduleName.lastIndexOf(QLatin1Char('/'));
    if (lastSlash >= 0) {
        moduleName = moduleName.mid(lastSlash + 1);
    }
    // Remove .ko, .ko.gz, .ko.xz, .ko.zst extensions
    moduleName.remove(QRegularExpression(QStringLiteral("\\.ko(\\.gz|\\.xz|\\.zst)?$")));

    ModuleInfo info = getModuleInfo(moduleName);

    // Check if this is an nvidia module
    bool isNvidia = moduleName == QStringLiteral("nvidia") ||
                    moduleName.startsWith(QStringLiteral("nvidia_")) ||
                    moduleName.startsWith(QStringLiteral("nvidia-"));

    // Provider - special handling for nvidia, otherwise use author or "Linux Foundation"
    if (isNvidia) {
        labelProviderValue_->setText(QStringLiteral("NVIDIA Corporation"));
    } else if (!info.author.isEmpty()) {
        labelProviderValue_->setText(info.author);
    } else {
        labelProviderValue_->setText(QStringLiteral("Linux Foundation"));
    }

    // Version
    if (!info.version.isEmpty()) {
        labelFileVersionValue_->setText(info.version);
    } else {
        // Fall back to kernel version
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            labelFileVersionValue_->setText(QString::fromLocal8Bit(buffer.release));
        }
    }

    // Copyright - special handling for nvidia, otherwise use license info
    if (isNvidia) {
        labelCopyrightValue_->setText(QStringLiteral("NVIDIA Driver License Agreement"));
    } else if (!info.license.isEmpty()) {
        labelCopyrightValue_->setText(info.license);
    } else {
        labelCopyrightValue_->clear();
    }

    // Digital signer
    if (isNvidia) {
        labelDigitalSignerValue_->setText(QStringLiteral("NVIDIA Corporation"));
    } else if (!info.signer.isEmpty()) {
        labelDigitalSignerValue_->setText(info.signer);
    } else {
        // Most kernel modules are signed by the kernel build
        labelDigitalSignerValue_->setText(QStringLiteral("Linux Foundation"));
    }
}

DriverDetailsDialog::ModuleInfo DriverDetailsDialog::getModuleInfo(const QString &moduleName) {
    ModuleInfo info;

    QProcess modinfo;
    modinfo.start(QStringLiteral("modinfo"), {moduleName});
    if (!modinfo.waitForFinished(3000)) {
        return info;
    }

    QString output = QString::fromUtf8(modinfo.readAllStandardOutput());
    QStringList lines = output.split(QStringLiteral("\n"), Qt::SkipEmptyParts);

    for (const QString &line : lines) {
        auto colonIdx = line.indexOf(QLatin1Char(':'));
        if (colonIdx < 0)
            continue;

        QString key = line.left(colonIdx).trimmed();
        QString value = line.mid(colonIdx + 1).trimmed();

        if (key == QStringLiteral("filename")) {
            info.filename = value;
        } else if (key == QStringLiteral("version")) {
            info.version = value;
        } else if (key == QStringLiteral("author")) {
            if (info.author.isEmpty()) {
                info.author = value;
            } else {
                info.author += QStringLiteral(", ") + value;
            }
        } else if (key == QStringLiteral("description")) {
            info.description = value;
        } else if (key == QStringLiteral("license")) {
            info.license = value;
        } else if (key == QStringLiteral("srcversion")) {
            info.srcversion = value;
        } else if (key == QStringLiteral("depends")) {
            info.depends = value;
        } else if (key == QStringLiteral("signer")) {
            info.signer = value;
        } else if (key == QStringLiteral("sig_key")) {
            info.sigKey = value;
        }
    }

    return info;
}
#else
void DriverDetailsDialog::onFileSelectionChanged() {
    labelProviderValue_->setText(tr("N/A"));
    labelFileVersionValue_->setText(tr("N/A"));
    labelCopyrightValue_->setText(tr("N/A"));
    labelDigitalSignerValue_->setText(tr("N/A"));
}

void DriverDetailsDialog::updateFileDetails([[maybe_unused]] const QString &modulePath) {
    // Not available on non-Linux platforms
}

DriverDetailsDialog::ModuleInfo
DriverDetailsDialog::getModuleInfo([[maybe_unused]] const QString &moduleName) {
    return ModuleInfo();
}
#endif
