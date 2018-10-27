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
    Command(pros::controller_id_e_t type, std::vector<int> controls) : type(type), controls(std::move(controls)) {
        allCommands.push_back(*this);
    }

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

    int GetValue(std::vector<std::pair<int, int>>& vec, int control) {
        for(std::pair<int, int>& t : vec) {
            if(t.first == control)
                return t.second;
        }
        throw "invalid control";
    }

    bool Contains(std::vector<int> &vec, int i) {
        for (int b : vec) {
            if (b == i)
                return true;
        }
        return false;
    }

    bool Contains(std::vector<int> &vec, std::vector<int> &i) {
        bool ret = true;
        for(int a : i) {
            if(!Contains(vec, a)) {
                ret = false;
            }
        }
        return ret;
    }

    void Update() {
        pros::Controller master(pros::E_CONTROLLER_MASTER);
        std::vector<int> newDigitalControls;

        std::vector<std::pair<int, int>> masterControls;
        // loop through all the values in the controller_digital_e_t enum
        for (int i = pros::E_CONTROLLER_DIGITAL_L1; i < pros::E_CONTROLLER_DIGITAL_A; i++) {
            auto button = static_cast<pros::controller_digital_e_t>(i);
            if (master.get_digital(button)) {
                newDigitalControls.push_back(button);
            }
        }
        // check for new presses and repeated presses
        for (int i : newDigitalControls) {
            if (Contains(digitalControlsCurrentlyPressed, i)) {
                // no fucking idea why but i get an error in the IDE when i use the DIGITAL_REPEAT here. Use it everywhere else pls
                masterControls.push_back({i, (1000 - 2)});
            } else {
                masterControls.push_back({i, (1000 - 1)});
            }
        }
        // check for releases
        for (int i : digitalControlsCurrentlyPressed) {
            if (!Contains(newDigitalControls, i)) {
                masterControls.push_back({i, 1000 - 3});
            }
        }
        digitalControlsCurrentlyPressed = newDigitalControls;
        for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i < pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
            auto analog = static_cast<pros::controller_analog_e_t>(i);
            int value = master.get_analog(analog);
            if (value != 0) {
                masterControls.push_back({analog, value});
            }
        }
        for(Command &command : Command::allCommands) {
            std::vector<std::pair<int, int>> controlsToSend;
            // determine which commands are active and then send the controls to their execute function
            for(std::pair<int, int> control : masterControls) {
                if(Contains(command.controls, control.first)) {
                    // the command is "active" and will be executed
                    controlsToSend.push_back(control);
                }
            }
            if(!controlsToSend.empty()) {
                //command.Execute(controlsToSend);
            }
        }
    }
}