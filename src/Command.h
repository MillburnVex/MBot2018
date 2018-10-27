#include <utility>
#include <vector>
#include "api.h"
#include <array>

#define DIGITAL_PRESSED = (1000 - 1)
#define DIGITAL_REPEATED = (1000 - 2)
#define DIGITAL_RELEASED = (1000 - 3)

/**
 * An abstract class that can be extended to create commands. On instantiation of a command, it is automatically added
 * to a static list which will be checked each tick with the latest values. Commands will be executed (each tick)
 * if any of their requested controls are not inactive. The best practice is to call only non-blocking methods inside
 * of the execute method, as blocking the thread will stop other commands from executing as well
 */
class Command {

public:
    static std::vector<Command> allCommands;

    std::vector<int> controls;
    pros::controller_id_e_t type;

    /**
     * @param type one of pros::E_CONTROLLER_{MASTER, PARTNER}
     * @param controls the controls this wants to be sent
     */
    Command(pros::controller_id_e_t type, std::vector<int> controls);

    /**
     * Executes the command with the given values. For analog controls, the value will be in [-127, 127]. For digital controls,
     * the value will be one of DIGITAL_{PRESSED, REPEATED, RELEASED}
     * @param values the control, control_value pairs that this command indicated it wanted to receive in the constructor. To
     * get a control_value, just use Commands::GetValue(pros::controller_{analog, digital}_e_t)
     */
    virtual void Execute(std::vector<std::pair<int, int>> &values) {}
};

namespace Commands {

    std::vector<Command> queue;
    std::vector<int> digitalControlsCurrentlyPressed;

    int GetValue(std::vector<std::pair<int, int>>& vec, int control);

    bool Contains(std::vector<int> &vec, int i);

    bool Contains(std::vector<int> &vec, std::vector<int> &i);

    void Update();
}