#include "Command.h"

class DriveCommand : public Command {
	DriveCommand() : Command(pros::E_CONTROLLER_MASTER, { pros::E_CONTROLLER_ANALOG_RIGHT_Y }) {}

	void Execute()
};

Command::Command(pros::controller_id_e_t type, std::vector<int> controls) : type(type), controls(std::move(controls))  {
    allCommands.push_back(*this);
}

int Commands::GetValue(std::vector<std::pair<int, int>>& vec, int control) {
    for(std::pair<int, int>& t : vec) {
        if(t.first == control)
            return t.second;
    }
    throw "invalid control";
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
    for(int a : i) {
        if(!Contains(vec, a)) {
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