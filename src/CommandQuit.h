#pragma once
#include "Command.h"
#include "Globals.h"
#include <QCoreApplication>

namespace Vasco {

class CommandQuit : public Command {
public:
    void execute() override {
        s_shouldQuit = true;
        clean_message_handler();
        QCoreApplication::quit();
    }

    QString name() const override {
        return "quit";
    }
};

}
