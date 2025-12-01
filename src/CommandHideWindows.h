#pragma once
#include "Command.h"
#include <QDebug>
#include <QGuiApplication>
#include <QWindow>

namespace Vasco {

class CommandHideWindows : public Command {
public:
    void execute() override {
        const auto windows = QGuiApplication::allWindows();
        qDebug() << Q_FUNC_INFO << "Hiding windows";
        for (auto window : windows) {
            window->hide();
            qDebug() << "Window:" << window->title() << "is now hidden.";
        }
    }

    QString name() const override {
        return "hide_windows";
    }
};

}
