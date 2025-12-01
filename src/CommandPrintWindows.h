#pragma once
#include "Command.h"
#include <QDebug>
#include <QGuiApplication>
#include <QWindow>

namespace Vasco {

class CommandPrintWindows : public Command {
public:
    void execute() override {
        const auto windows = QGuiApplication::allWindows();
        qDebug() << Q_FUNC_INFO << "Printing count=" << windows.size();
        for (auto window : windows) {
            qDebug() << "Window:" << window->title() << ";isVisible=" << window->isVisible() << "; this=" << window
                     << "; geometry=" << window->geometry();
        }
    }

    QString name() const override {
        return "print_windows";
    }
};

}
