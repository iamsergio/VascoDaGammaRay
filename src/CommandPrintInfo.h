#pragma once
#include "Command.h"
#include <QDebug>
#include <QStandardPaths>
#include <QProcessEnvironment>

namespace Vasco {

class CommandPrintInfo : public Command {
public:
    void execute() override {
        qDebug() << Q_FUNC_INFO << "Printing QStandardPaths locations";

        const QList<QStandardPaths::StandardLocation> locationsList = {
            QStandardPaths::DesktopLocation,
            QStandardPaths::DocumentsLocation,
            QStandardPaths::FontsLocation,
            QStandardPaths::ApplicationsLocation,
            QStandardPaths::MusicLocation,
            QStandardPaths::MoviesLocation,
            QStandardPaths::PicturesLocation,
            QStandardPaths::TempLocation,
            QStandardPaths::HomeLocation,
            QStandardPaths::AppLocalDataLocation,
            QStandardPaths::CacheLocation,
            QStandardPaths::GenericDataLocation,
            QStandardPaths::RuntimeLocation,
            QStandardPaths::ConfigLocation,
            QStandardPaths::DownloadLocation,
            QStandardPaths::GenericCacheLocation,
            QStandardPaths::GenericConfigLocation,
            QStandardPaths::AppDataLocation,
            QStandardPaths::AppConfigLocation,
            QStandardPaths::PublicShareLocation,
            QStandardPaths::TemplatesLocation,
#if (QT_VERSION >= QT_VERSION_CHECK(6, 7, 0))
            QStandardPaths::StateLocation,
            QStandardPaths::GenericStateLocation
#endif
        };

        for (auto type : locationsList) {
            const auto paths = QStandardPaths::standardLocations(type);
            qDebug() << type << "standard:" << paths;
            qDebug() << type << "writable:" << QStandardPaths::writableLocation(type);
        }

        const auto envVars = QProcessEnvironment::systemEnvironment().toStringList();
        qDebug() << "Environment Variables:\n";
        for (const auto &envVar : envVars) {
            qDebug().noquote() << envVar;
        }
    }

    QString name() const override {
        return "print_info";
    }
};

}
