#pragma once
#include "Command.h"
#include "Globals.h"
#include <QDebug>

namespace Vasco {

class CommandTrackWindowEvents : public Command {
public:
    void execute() override {
        s_shouldTrackWindowEvents = true;
        qDebug() << Q_FUNC_INFO << "Window event dumping enabled.";
    }

    QString name() const override {
        return "track_window_events";
    }
};

}
