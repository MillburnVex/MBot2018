#include <vector>
#include <api.h>

#define DIGITAL_PRESSED = (1000 - 1)
#define DIGITAL_REPEATED = (1000 - 2)
#define DIGITAL_RELEASED = (1000 - 3)

class Command {
private:
    int controls[];
    pros::controller_id_e_t type;
public:
    static std::vector<Command> allCommands;

    Command(pros::controller_id_e_t type, int controls[]) : type(type), controls(controls) {
        allCommands.push_back(*this);
    }

    /**
     * Executes the command with the given values. For analog controls, the value will be in [-127, 127]. For digital controls,
     * the value will be one of DIGITAL_{PRESSED, REPEATED, RELEASED}.
     * @param values
     */
    virtual void execute(std::pair<int, int> values[]) = 0;
};

namespace Commands {

    std::vector<Command> queue;
    std::vector<int> digitalControlsCurrentlyPressed;

    bool contains(std::vector<int>& vec, int i) {
        for (int b : vec) {
            if (b == i)
                return true;
        }
        return false;
    }

    bool contains(std::vector<std::pair<int, int>>& vec, std::pair<int, int>& pair) {

    }

    void update() {
        pros::Controller master(pros::E_CONTROLLER_MASTER);
        std::vector<std::pair<int, int>> masterControls;
        std::vector<int> newDigitalControls;
        // loop through all the values in the controller_digital_e_t enum
        for (int i = pros::E_CONTROLLER_DIGITAL_L1; i < pros::E_CONTROLLER_DIGITAL_A; i++) {
            auto button = static_cast<pros::controller_digital_e_t>(i);
            if (master.get_digital(button)) {
                newDigitalControls.push_back(button);
            }
        }
        // check for new presses and repeated presses
        for (int i : newDigitalControls) {
            if (contains(digitalControlsCurrentlyPressed, i)) {
                // no fucking idea why but i get an error in the IDE when i use the DIGITAL_REPEAT here. Use it everywhere else pls
                masterControls.emplace_back({i, (1000 - 2)});
            } else {
                masterControls.emplace_back({i, (1000 - 1)});
            }
        }
        // check for releases
        for (int i : digitalControlsCurrentlyPressed) {
            if (!contains(newDigitalControls, i)) {
                masterControls.emplace_back({i, 1000 - 3});
            }
        }
        for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i < pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
            auto analog = static_cast<pros::controller_analog_e_t>(i);
            int value = master.get_analog(analog);
            if (value != 0) {
                masterControls.emplace_back({analog, value});
            }
        }
        auto c = Command(pros::E_CONTROLLER_MASTER, {});
        for(Command command : Command::allCommands) {
            // determine which commands are active and then send the controls to their execute function
        }
    }
}