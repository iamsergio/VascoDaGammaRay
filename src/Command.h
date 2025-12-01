#pragma once
#include <QString>
#include <memory>

namespace Vasco {

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual QString name() const = 0;

    static std::unique_ptr<Command> create(const QString &commandName);
};

}
