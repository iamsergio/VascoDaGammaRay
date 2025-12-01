#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLocalSocket>
#include <QDir>
#include <QDebug>
#include <QCommandLineParser>
#include <QTextStream>

void sendCommand(const QString &socketName, const QString &command) {
    QLocalSocket socket;
    socket.connectToServer(socketName);
    if (socket.waitForConnected(1000)) {
        socket.write(command.toUtf8());
        socket.waitForBytesWritten(1000);
        socket.disconnectFromServer();
        qDebug() << "Sent command:" << command;
    } else {
        qWarning() << "Failed to connect to" << socketName << ":" << socket.errorString();
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("vasco-gui");
    QApplication::setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Vasco Da GammaRay GUI");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("socket", "The socket name to connect to.");

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.isEmpty()) {
        QDir tmpDir("/tmp");
        QStringList filters;
        filters << "*-IpcPipe";
        tmpDir.setNameFilters(filters);

        QFileInfoList list = tmpDir.entryInfoList(QDir::System | QDir::Files);
        
        if (list.isEmpty()) {
            QTextStream(stdout) << "No Vasco sockets found in /tmp/" << Qt::endl;
        } else {
            QTextStream(stdout) << "Found Vasco sockets:" << Qt::endl;
            for (const QFileInfo &fileInfo : list) {
                QTextStream(stdout) << fileInfo.fileName() << Qt::endl;
            }
        }
        return 0;
    }

    QString socketName = args.first();

    QWidget window;
    window.setWindowTitle("Vasco Controller: " + socketName);
    QVBoxLayout *layout = new QVBoxLayout(&window);

    auto addButton = [&](const QString &text, const QString &cmd) {
        QPushButton *btn = new QPushButton(text);
        QObject::connect(btn, &QPushButton::clicked, [socketName, cmd]() {
            sendCommand(socketName, cmd);
        });
        layout->addWidget(btn);
    };

    addButton("Quit", "quit");
    addButton("Print Windows", "print_windows");
    addButton("Hide Windows", "hide_windows");
    addButton("Set Persistent Windows False", "set_persistent_windows_false");
    addButton("Print Info", "print_info");
    addButton("Track Window Events", "track_window_events");

    window.show();

    return app.exec();
}
