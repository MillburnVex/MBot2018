#include "Command.h"
#include "BotComponent.h"

std::vector<Command*> Command::allCommands;
std::vector<int> Command::digitalControlsCurrentlyPressed;

class DriveCommands : public Command {
public:
    DriveCommands() : Command(pros::E_CONTROLLER_MASTER,
                              {pros::E_CONTROLLER_ANALOG_RIGHT_Y, pros::E_CONTROLLER_ANALOG_RIGHT_X}) {}

    void Execute(std::vector<std::pair<int, int>> &values) override {
        int linear = Commands::GetValue(values, pros::E_CONTROLLER_ANALOG_RIGHT_Y);
        int rotation = Commands::GetValue(values, pros::E_CONTROLLER_ANALOG_RIGHT_X);
        Components::Execute(ComponentAction::DRIVE_LINEAR, linear);
        Components::Execute(ComponentAction::DRIVE_ROTATE, rotation);
    }
};

class BallLiftCommands : public Command {
public:
    BallLiftCommands() : Command(pros::E_CONTROLLER_MASTER,
                             {pros::E_CONTROLLER_DIGITAL_R1, pros::E_CONTROLLER_DIGITAL_R2}) {}

    void Execute(std::vector<std::pair<int, int>> &values) override {
        int up = (Commands::GetValue(values, pros::E_CONTROLLER_DIGITAL_R1) == DIGITAL_PRESSED);
        int down = (Commands::GetValue(values, pros::E_CONTROLLER_DIGITAL_R2) == DIGITAL_PRESSED);
        if(Commands::GetValue(values, pros::E_CONTROLLER_DIGITAL_R1) == DIGITAL_PRESSED) {

        }
    }
};

void Commands::Init() {
    new DriveCommands();
    new BallLiftCommands();
}

Command::Command(pros::controller_id_e_t type, std::vector<int> controls) : type(type), controls(std::move(controls)) {
    Command::allCommands.push_back(this);
}

int Commands::GetValue(std::vector<std::pair<int, int>> &vec, int control) {
    for (std::pair<int, int> &t : vec) {
        if (t.first == control)
            return t.second;
    }
    return CONTROL_NOT_ACTIVE;
}

bool Commands::Contains(std::vector<int> &vec, int i) {
    for (int b : vec) {
        if (b == i)
            return true;
    }
    return false;
}

bool Commands::Contains(std::vector<int> &vec, std::vector<int> &i) {
    bool ret = true;
    for (int a : i) {
        if (!Contains(vec, a)) {
            ret = false;
        }
    }
    return ret;
}

void Commands::Update() {
    pros::Controller master(pros::E_CONTROLLER_MASTER);
    std::vector<int> newDigitalControls;

    std::vector<std::pair<int, int>> masterControls;
    // loop through all the values in the controller_digital_e_t enum
    for (int i = pros::E_CONTROLLER_DIGITAL_L1; i < pros::E_CONTROLLER_DIGITAL_A; i++) {
        auto button = static_cast<pros::controller_digital_e_t>(i);
        if (master.get_digital(button)) {
            pros::lcd::print(1, std::to_string(button).c_str());
            newDigitalControls.push_back(button);
        }
    }
    // check for new presses and repeated presses
    for (int i : newDigitalControls) {
        if (Contains(Command::digitalControlsCurrentlyPressed, i)) {
            masterControls.push_back({i, DIGITAL_REPEATED});
        } else {
            masterControls.push_back({i, DIGITAL_PRESSED});
        }
    }
    // check for releases
    for (int i : Command::digitalControlsCurrentlyPressed) {
        if (!Contains(newDigitalControls, i)) {
            masterControls.push_back({i, DIGITAL_RELEASED});
        }
    }
    Command::digitalControlsCurrentlyPressed = newDigitalControls;
    for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i < pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
        auto analog = static_cast<pros::controller_analog_e_t>(i);
        int value = master.get_analog(analog);
        if (value != 0) {
            masterControls.push_back({analog, value});
        }
    }
    for (Command *command : Command::allCommands) {
        std::vector<std::pair<int, int>> controlsToSend;
        // determine which commands are active and then send the controls to their execute function
        for (std::pair<int, int> control : masterControls) {
            if (Contains(command->controls, control.first)) {
                // the command is "active" and will be executed
                controlsToSend.push_back(control);
            }
        }
        if (!controlsToSend.empty()) {
            command->Execute(controlsToSend);
        }
    }
}