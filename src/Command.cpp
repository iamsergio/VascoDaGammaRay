#include "Command.h"
#include "CommandQuit.h"
#include "CommandPrintWindows.h"
#include "CommandHideWindows.h"
#include "CommandSetPersistentWindowsFalse.h"
#include "CommandPrintInfo.h"
#include "CommandTrackWindowEvents.h"

namespace Vasco {

std::unique_ptr<Command> Command::create(const QString &commandName)
{
    if (commandName == "quit") {
        return std::make_unique<CommandQuit>();
    } else if (commandName == "print_windows") {
        return std::make_unique<CommandPrintWindows>();
    } else if (commandName == "hide_windows") {
        return std::make_unique<CommandHideWindows>();
    } else if (commandName == "set_persistent_windows_false") {
        return std::make_unique<CommandSetPersistentWindowsFalse>();
    } else if (commandName == "print_info") {
        return std::make_unique<CommandPrintInfo>();
    } else if (commandName == "track_window_events") {
        return std::make_unique<CommandTrackWindowEvents>();
    }
    
    return nullptr;
}

}
