// SPDX-License-Identifier: MIT

#include "lib.h"

#include <QDebug>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QGuiApplication>
#include <QWindow>
#include <QQuickWindow>
#include <QTextStream>
#include <QProcessEnvironment>

#include <thread>

static bool s_shouldQuit = false;

namespace Vasco {

QStringList availableCommands()
{
    return QStringList() << "quit" << "print_windows" << "hide_windows" << "set_persistent_windows_false" << "print_info";
}

QtMessageHandler s_originalHandler;
QFile *s_logFile = nullptr;

void install_qt_message_handler()
{
    if (qEnvironmentVariableIsSet("VASCO_OUTPUT_FILE")) {
        const QString outputFilePath = qEnvironmentVariable("VASCO_OUTPUT_FILE");
        s_logFile = new QFile(outputFilePath);

        if (!s_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            delete s_logFile;
            s_logFile = nullptr;
            qWarning() << "Failed to open output file for logging:" << outputFilePath;
        }

        s_originalHandler = qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
            if (s_logFile) {
                static QTextStream out(s_logFile);
                out << msg << Qt::endl;
            }

            s_originalHandler(type, context, msg);
        });
    }
}

void wait_for_qt()
{
    while (!QCoreApplication::instance()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    QMetaObject::invokeMethod(QCoreApplication::instance(), [] {
        install_qt_message_handler();
    });

    qDebug() << "Qt started";
}

void command_printWindows()
{
    const auto windows = QGuiApplication::allWindows();
    qDebug() << Q_FUNC_INFO << "Printing count=" << windows.size();
    for (auto window : windows) {
        qDebug() << "Window:" << window->title() << ";isVisible=" << window->isVisible() << "; this=" << window
                 << "; geometry=" << window->geometry();
    }
}

void command_hideWindows()
{
    const auto windows = QGuiApplication::allWindows();
    qDebug() << Q_FUNC_INFO << "Hiding windows";
    for (auto window : windows) {
        window->hide();
        qDebug() << "Window:" << window->title() << "is now hidden.";
    }
}

void command_setPersistentWindowsFalse()
{
    const auto windows = QGuiApplication::allWindows();
    qDebug() << Q_FUNC_INFO << "Setting persistent to false for QQuickWindows";
    for (auto window : windows) {
        if (auto quickWindow = dynamic_cast<QQuickWindow *>(window)) {
            quickWindow->setPersistentSceneGraph(false);
            qDebug() << "QQuickWindow:" << quickWindow->title() << "persistent set to false.";
        }
    }
}

void command_printInfo()
{
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

    for (auto location : locationsList) {
        const auto paths = QStandardPaths::standardLocations(location);
        qDebug() << "Location:" << location << "Paths:" << paths;
    }

    const auto envVars = QProcessEnvironment::systemEnvironment().toStringList();
    qDebug() << "Environment Variables:\n";
    for (const auto &envVar : envVars) {
        qDebug().noquote() << envVar;
    }
}


void handleCommand(const QByteArray &command)
{
    if (command.size() > 100) {
        qWarning() << Q_FUNC_INFO << "Weird command received";
        return;
    }

    qDebug() << Q_FUNC_INFO << "received" << command;
    if (command == "quit") {
        s_shouldQuit = true;
        QCoreApplication::quit();
    } else if (command == "print_windows") {
        QMetaObject::invokeMethod(QCoreApplication::instance(), command_printWindows);
    } else if (command == "hide_windows") {
        QMetaObject::invokeMethod(QCoreApplication::instance(), command_hideWindows);
    } else if (command == "print_info") {
        QMetaObject::invokeMethod(QCoreApplication::instance(), command_printInfo);
    } else {
        qWarning() << Q_FUNC_INFO << "Unknown command received. Available commands: " << availableCommands().join(",");
    }
}

QString appName()
{
    QFileInfo info(QCoreApplication::applicationFilePath());
    return info.fileName();
}

bool listen()
{
    QLocalServer server;

    const QString pipeName = QStringLiteral("%1-IpcPipe").arg(appName());
    if (!server.listen(pipeName)) {
        qWarning() << "Failed to start server:" << server.errorString() << "on pipe" << pipeName;
        return false;
    }
    qDebug().nospace().noquote() << "Server listening on /tmp/" << pipeName;

    while (server.isListening()) {
        if (server.waitForNewConnection(-1)) {
            if (QLocalSocket *client = server.nextPendingConnection()) {
                client->waitForReadyRead();
                handleCommand(client->readAll());
                client->close();
            }
        }

        if (s_shouldQuit || !QCoreApplication::instance())
            break;
    }

    return true;
}

void library_init()
{
    qDebug() << "Library loaded successfully!";
    std::thread([]() {
        qDebug() << "Thread started!";
        Vasco::wait_for_qt();
        Vasco::listen();
    }).detach();
}

struct LibraryLoader
{
    LibraryLoader()
    {
        library_init();
    }
};

static LibraryLoader loader;
}
