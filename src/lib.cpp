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
#include <QQmlDebuggingEnabler>
#include <QTimer>
#include <QEvent>
#include <QDateTime>

#include <QtCore/private/qhooks_p.h>

#include <array>
#include <thread>

#include "Globals.h"
#include "Command.h"

namespace Vasco {

bool s_shouldQuit = false;
bool s_shouldTrackWindowEvents = true;

thread_local QHash<QObject *, qint64> s_objectCreationTimes;
QVector<QQuickWindow *> s_seenWindows;

QString currentTimestamp()
{
    return QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
}

void connectWindowDebugInfo(QWindow *window) {

    QObject::connect(window, &QWindow::screenChanged, window, [window](QScreen *screen) {
        if (s_shouldTrackWindowEvents)
            qDebug() << currentTimestamp() << "[Window]" << window << "screenChanged" << screen;
    });

    QObject::connect(window, &QWindow::windowStateChanged, window, [window](Qt::WindowState state) {
        if (s_shouldTrackWindowEvents)
            qDebug() << currentTimestamp() << "[Window]" << window << "windowStateChanged" << state;
    });

    QObject::connect(window, &QWindow::transientParentChanged, window, [window]() {
        if (s_shouldTrackWindowEvents)
            qDebug() << currentTimestamp() << "[Window]" << window << "transientParentChanged" << window->transientParent();
    });

    QObject::connect(window, &QWindow::visibleChanged, window, [window]() {
        if (s_shouldTrackWindowEvents)
            qDebug() << currentTimestamp() << "[Window]" << window << "visibleChanged" << window->isVisible();
    });

    QObject::connect(window, &QWindow::visibilityChanged, window, [window](QWindow::Visibility visibility) {
        if (s_shouldTrackWindowEvents)
            qDebug() << currentTimestamp() << "[Window]" << window << "visibilityChanged" << visibility;
    });
}

void debugWindowDebugInfo(QWindow *window, QEvent *event)  {
    if (!s_shouldTrackWindowEvents)
        return;

    static std::array<QEvent::Type, 4> interestingEvents = {
        QEvent::Expose,
        QEvent::Resize,
        QEvent::Move,
        QEvent::ParentChange
    };

    if (std::find(interestingEvents.begin(), interestingEvents.end(), event->type()) == interestingEvents.end())
        return;

    if (event->type() == QEvent::Expose) {
        QExposeEvent *exposeEvent = static_cast<QExposeEvent *>(event);
        qDebug() << currentTimestamp() << "[Window]" << window << "Event:" << event->type()
                 << "isExposed:" << window->isExposed();
        
    } else {
        qDebug() << currentTimestamp() << "[Window]" << window << "Event:" << event->type(); 
    }
}

class EventFilter : public QObject
{
public:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (auto window = qobject_cast<QQuickWindow *>(obj)) {

            if (s_shouldTrackWindowEvents)
                debugWindowDebugInfo(window, event);

            if (!s_seenWindows.contains(window)) {
                s_seenWindows.append(window);
                auto ctx = new QObject();
                QObject::connect(window, &QQuickWindow::frameSwapped, ctx, [window, ctx] {
                    delete ctx;
                    auto it = s_objectCreationTimes.find(window);
                    if (it != s_objectCreationTimes.end()) {
                        qint64 currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                 std::chrono::steady_clock::now().time_since_epoch())
                                                 .count();
                        qint64 elapsedTime = currentTime - it.value();
                        qDebug() << "Frame swapped for window" << window << "elapsed time since creation:" << elapsedTime << "ms";
                    }
                });

                connectWindowDebugInfo(window);
            }
        }

        return QObject::eventFilter(obj, event);
    }
};

static void objectAddHook(QObject *obj)
{
    s_objectCreationTimes[obj] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::steady_clock::now().time_since_epoch())
                                     .count();
}

static void objectRemoveHook(QObject *obj)
{
    s_objectCreationTimes.remove(obj);
}

QString appName()
{
    QFileInfo info(QCoreApplication::applicationFilePath());
    return info.fileName();
}

QString loggingFileName()
{
    // TODO: Add some env var
    return QStringLiteral("/tmp/vasco-%1.out").arg(appName());
}

QtMessageHandler s_originalHandler;
QFile *s_logFile = nullptr;

void install_qt_message_handler()
{
    const QString outputFilePath = loggingFileName();
    if (!outputFilePath.isEmpty()) {
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

            static const QStringList blacklist = { "QSocketNotifier: Can only be used with threads started with QThread" };
            if (std::any_of(blacklist.begin(), blacklist.end(), [&](const QString &blacklistedMsg) { return msg.contains(blacklistedMsg); })) {
                return;
            }

            s_originalHandler(type, context, msg);
        });
    }
}

void clean_message_handler()
{
    delete s_logFile;
    s_logFile = nullptr;
}

void wait_for_qt()
{
    while (!QCoreApplication::instance()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    QTimer::singleShot(0, QCoreApplication::instance(), []() {
        QCoreApplication::instance()->installEventFilter(new EventFilter());
    });

    install_qt_message_handler();
    qDebug() << "vasco: Qt started. Logging to" << loggingFileName();
}

void handleCommand(const QByteArray &commandName)
{
    if (commandName.size() > 100) {
        qWarning() << Q_FUNC_INFO << "Weird command received";
        return;
    }

    qDebug() << Q_FUNC_INFO << "received" << commandName;

    auto cmd = Command::create(commandName);
    if (cmd) {
        // We need to move the unique_ptr into the lambda, but C++14/17 lambdas with move capture
        // are a bit verbose or we can just release it to a raw pointer since we delete it inside.
        // Or better, use a shared_ptr or just release.
        // The previous code was releasing it.
        Command *rawCmd = cmd.release();
        QMetaObject::invokeMethod(QCoreApplication::instance(), [rawCmd]() {
            rawCmd->execute();
            delete rawCmd;
        });
    } else {
        qWarning() << Q_FUNC_INFO << "Unknown command received:" << commandName;
    }
}

bool listen()
{
    QLocalServer server;

    const QString pipeName = QStringLiteral("%1-IpcPipe").arg(appName());
    if (!server.listen(pipeName)) {
        qWarning() << "Failed to start server:" << server.errorString() << "on pipe" << pipeName;
        return false;
    }
    qDebug().nospace().noquote() << "Vasco: Server listening on /tmp/" << pipeName;

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
    QQmlDebuggingEnabler::enableDebugging(true);
    qtHookData[QHooks::AddQObject] = ( quintptr )&Vasco::objectAddHook;
    qtHookData[QHooks::RemoveQObject] = ( quintptr )&Vasco::objectRemoveHook;

    std::thread([]() {
        qDebug() << "Vasco: Started";

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
