#include "driverinfo.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QSettings>

#include <softpub.h>
#include <wincrypt.h>
#include <windows.h>
#include <wintrust.h>

DriverSearchResult findDriverFiles(const QString &driverName) {
    DriverSearchResult result;

    // Check registry for driver image path
    QString regPath =
        QStringLiteral("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\") + driverName;
    QSettings reg(regPath, QSettings::NativeFormat);

    QString imagePath = reg.value(QStringLiteral("ImagePath")).toString();
    if (!imagePath.isEmpty()) {
        // Expand environment variables and normalize path
        if (imagePath.startsWith(QStringLiteral("\\SystemRoot\\"))) {
            imagePath.replace(QStringLiteral("\\SystemRoot\\"),
                              QDir::toNativeSeparators(QDir::rootPath()) +
                                  QStringLiteral("Windows\\"));
        } else if (imagePath.startsWith(QStringLiteral("System32\\")) ||
                   imagePath.startsWith(QStringLiteral("system32\\"))) {
            imagePath = QDir::toNativeSeparators(QDir::rootPath()) + QStringLiteral("Windows\\") +
                        imagePath;
        } else if (imagePath.startsWith(QStringLiteral("\\??\\"))) {
            imagePath = imagePath.mid(4);
        }

        imagePath = QDir::fromNativeSeparators(imagePath);

        if (QFile::exists(imagePath)) {
            result.paths << imagePath;
        }
    }

    // If not found in registry, check common driver locations
    if (result.paths.isEmpty()) {
        QString sysName = driverName;
        if (!sysName.endsWith(QStringLiteral(".sys"), Qt::CaseInsensitive)) {
            sysName += QStringLiteral(".sys");
        }

        QStringList searchDirs = {
            QDir::rootPath() + QStringLiteral("Windows/System32/drivers"),
            QDir::rootPath() + QStringLiteral("Windows/System32/DriverStore/FileRepository"),
        };

        for (const QString &searchDir : searchDirs) {
            QString directPath = searchDir + QLatin1Char('/') + sysName;
            if (QFile::exists(directPath)) {
                result.paths << directPath;
                break;
            }
        }

        // Search DriverStore subdirectories if not found directly
        if (result.paths.isEmpty()) {
            QString driverStore =
                QDir::rootPath() + QStringLiteral("Windows/System32/DriverStore/FileRepository");
            QDirIterator it(driverStore, {sysName}, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                result.paths << it.next();
                break;
            }
        }
    }

    return result;
}

DriverInfo getDriverInfo(const QString &driverPath) {
    DriverInfo info;
    info.filename = driverPath;

    std::wstring wpath = driverPath.toStdWString();

    // Get file version info size
    DWORD dummy;
    DWORD versionInfoSize = GetFileVersionInfoSizeW(wpath.c_str(), &dummy);

    if (versionInfoSize > 0) {
        QByteArray versionData(static_cast<int>(versionInfoSize), '\0');

        if (GetFileVersionInfoW(wpath.c_str(), 0, versionInfoSize, versionData.data())) {
            struct LANGANDCODEPAGE {
                WORD wLanguage;
                WORD wCodePage;
            } *lpTranslate;
            UINT cbTranslate;

            if (VerQueryValueW(versionData.data(),
                               L"\\VarFileInfo\\Translation",
                               reinterpret_cast<LPVOID *>(&lpTranslate),
                               &cbTranslate) &&
                cbTranslate >= sizeof(LANGANDCODEPAGE)) {

                wchar_t subBlock[256];

                // Company name (provider)
                swprintf_s(subBlock,
                           L"\\StringFileInfo\\%04x%04x\\CompanyName",
                           lpTranslate[0].wLanguage,
                           lpTranslate[0].wCodePage);
                wchar_t *companyName = nullptr;
                UINT len;
                if (VerQueryValueW(versionData.data(),
                                   subBlock,
                                   reinterpret_cast<LPVOID *>(&companyName),
                                   &len) &&
                    len > 0) {
                    info.author = QString::fromWCharArray(companyName);
                }

                // File version
                swprintf_s(subBlock,
                           L"\\StringFileInfo\\%04x%04x\\FileVersion",
                           lpTranslate[0].wLanguage,
                           lpTranslate[0].wCodePage);
                wchar_t *fileVersion = nullptr;
                if (VerQueryValueW(versionData.data(),
                                   subBlock,
                                   reinterpret_cast<LPVOID *>(&fileVersion),
                                   &len) &&
                    len > 0) {
                    info.version = QString::fromWCharArray(fileVersion);
                }

                // Legal copyright
                swprintf_s(subBlock,
                           L"\\StringFileInfo\\%04x%04x\\LegalCopyright",
                           lpTranslate[0].wLanguage,
                           lpTranslate[0].wCodePage);
                wchar_t *copyright = nullptr;
                if (VerQueryValueW(versionData.data(),
                                   subBlock,
                                   reinterpret_cast<LPVOID *>(&copyright),
                                   &len) &&
                    len > 0) {
                    info.license = QString::fromWCharArray(copyright);
                }

                // File description
                swprintf_s(subBlock,
                           L"\\StringFileInfo\\%04x%04x\\FileDescription",
                           lpTranslate[0].wLanguage,
                           lpTranslate[0].wCodePage);
                wchar_t *description = nullptr;
                if (VerQueryValueW(versionData.data(),
                                   subBlock,
                                   reinterpret_cast<LPVOID *>(&description),
                                   &len) &&
                    len > 0) {
                    info.description = QString::fromWCharArray(description);
                }
            }
        }
    }

    // Get digital signature info using WinVerifyTrust
    WINTRUST_FILE_INFO fileInfo;
    memset(&fileInfo, 0, sizeof(fileInfo));
    fileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    fileInfo.pcwszFilePath = wpath.c_str();

    GUID policyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    WINTRUST_DATA trustData;
    memset(&trustData, 0, sizeof(trustData));
    trustData.cbStruct = sizeof(WINTRUST_DATA);
    trustData.dwUIChoice = WTD_UI_NONE;
    trustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    trustData.dwUnionChoice = WTD_CHOICE_FILE;
    trustData.pFile = &fileInfo;
    trustData.dwStateAction = WTD_STATEACTION_VERIFY;

    LONG status = WinVerifyTrust(nullptr, &policyGUID, &trustData);

    if (status == ERROR_SUCCESS) {
        CRYPT_PROVIDER_DATA *provData = WTHelperProvDataFromStateData(trustData.hWVTStateData);
        if (provData) {
            CRYPT_PROVIDER_SGNR *signer = WTHelperGetProvSignerFromChain(provData, 0, FALSE, 0);
            if (signer && signer->pasCertChain && signer->csCertChain > 0) {
                PCCERT_CONTEXT cert = signer->pasCertChain[0].pCert;
                if (cert) {
                    wchar_t subjectName[256];
                    DWORD subjectLen = CertGetNameStringW(
                        cert, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, nullptr, subjectName, 256);
                    if (subjectLen > 1) {
                        info.signer = QString::fromWCharArray(subjectName);
                    }
                }
            }
        }
    }

    trustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(nullptr, &policyGUID, &trustData);

    return info;
}
