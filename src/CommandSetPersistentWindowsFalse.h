#pragma once
#include "Command.h"
#include <QDebug>
#include <QGuiApplication>
#include <QQuickWindow>

namespace Vasco {

class CommandSetPersistentWindowsFalse : public Command {
public:
    void execute() override {
        const auto windows = QGuiApplication::allWindows();
        qDebug() << Q_FUNC_INFO << "Setting persistent to false for QQuickWindows";
        for (auto window : windows) {
            if (auto quickWindow = dynamic_cast<QQuickWindow *>(window)) {
                quickWindow->setPersistentSceneGraph(false);
                qDebug() << "QQuickWindow:" << quickWindow->title() << "persistent set to false.";
            }
        }
    }

    QString name() const override {
        return "set_persistent_windows_false";
    }
};

}
