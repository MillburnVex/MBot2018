#include <vector>
#include <algorithm>
#include "Command.h"
#include "BotComponent.h"

std::vector<Command *> Command::allCommands;
std::vector<int> Command::controlsLastActive;
std::vector<ControlPress*> Command::executedControls;

class DriveCommands : public Command {
public:
    DriveCommands() : Command(pros::E_CONTROLLER_MASTER,
                              {Control::C_DRIVE_LINEAR, Control::C_DRIVE_ROTATE}) {}

    void Execute(std::vector<ControlPress *> &values) override {
        int linear = Commands::GetValue(values, Control::C_DRIVE_LINEAR);
        int rotation = Commands::GetValue(values, Control::C_DRIVE_ROTATE);
        Components::Execute(ActionType::DRIVE_LINEAR, linear);
        Components::Execute(ActionType::DRIVE_ROTATE, rotation);
    }
};

class BallLiftCommands : public Command {
public:
    BallLiftCommands() : Command(pros::E_CONTROLLER_MASTER,
                                 {Control::C_BALL_LIFT_DOWN, Control::C_BALL_LIFT_UP}) {}

    void Execute(std::vector<ControlPress *> &values) override {
		printf("%d\n", Commands::GetPressType(values, Control::C_BALL_LIFT_UP));
        bool up = (Commands::GetPressType(values, Control::C_BALL_LIFT_UP) == PressType::PRESSED);
        bool down = (Commands::GetPressType(values, Control::C_BALL_LIFT_DOWN) == PressType::PRESSED);
        // here's where we check sensor values
		printf("is it htis oje?\n");
        if(!up && !down) {
			printf("this one\n");

            Components::Execute(ActionType::BALL_LIFT_UP);
        } else if(up) {

            Components::Execute(ActionType::BALL_LIFT_UP);
        } else {
            Components::Execute(ActionType::BALL_LIFT_DOWN);
        }
    }
};

class ShootCommand : public Command {
public:
    ShootCommand() : Command(pros::E_CONTROLLER_MASTER, {
            Control::C_SHOOT
    }) {}

    void Execute(std::vector<ControlPress *> &values) override {
        bool shoot = (Commands::GetPressType(values, Control::C_SHOOT) == PressType::PRESSED);
        if(shoot) {
            // ok bazinga we going
        }
    }
};

void Commands::Init() {
    new DriveCommands();
    new BallLiftCommands();
	new ShootCommand();
}

Command::Command(pros::controller_id_e_t type, std::vector<int> controls) : type(type), controls(std::move(controls)) {
    Command::allCommands.push_back(this);
}

int Commands::GetValue(std::vector<ControlPress *> &vec, int control) {
    for (ControlPress *t : vec) {
        if (t->control == control)
            return t->value;
    }
    return CONTROL_NOT_ACTIVE;
}

PressType Commands::GetPressType(std::vector<ControlPress *> &vec, int control) {
    for (ControlPress *t : vec) {
        if (t->control == control)
            return t->pressType;
    }
	return PressType::NOT_ACTIVE;
}

bool Commands::Contains(std::vector<int> &vec, int i) {
	return std::find(vec.begin(), vec.end(), i) != vec.end();
}

bool Commands::Contains(std::vector<ControlPress *> presses, int control) {
    for (ControlPress *press : presses) {
        if (press->control == control) {
            return true;
        }
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
    pros::Controller partner(pros::E_CONTROLLER_PARTNER);
    std::vector<ControlPress *> newControls;

    std::vector<ControlPress *> masterControls;
	std::vector<ControlPress *> controlsToSend;


    // loop through all the values in the controller_digital_e_t enum
    for (int i = pros::E_CONTROLLER_DIGITAL_L1; i <= pros::E_CONTROLLER_DIGITAL_A; i++) {
        auto button = static_cast<pros::controller_digital_e_t>(i);
		
        if (master.get_digital(button)) {
			
            ControlPress press = {};
            press.control = button;
            press.value = 1;
            // press type assigned later
			newControls.push_back(&press);
        }
    }

    // loop through all the values in the controller_analog_e_t enum
    for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i <= pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
        auto control = static_cast<pros::controller_analog_e_t>(i);
        int value = master.get_analog(control);
        if (value < -ANALOG_CONTROL_ACTIVE_THRESHOLD || value > ANALOG_CONTROL_ACTIVE_THRESHOLD) {
            ControlPress press = {};
            press.control = control;
            press.value = value;
            // press type assigned later
			newControls.push_back(&press);
        }
    }

    // check for new presses and repeated presses
    for (ControlPress *i : newControls) {
        // assign press type
        if (Contains(Command::controlsLastActive, i->control)) {
            i->pressType = PressType::REPEATED;
        } else {
            i->pressType = PressType::PRESSED;
        }
        masterControls.push_back(i);
    }

    // check for releases
    for (int i : Command::controlsLastActive) {
        if (!Contains(newControls, i)) {
            ControlPress press = {};
            press.pressType = PressType::RELEASED;
            press.control = i;
            press.value = 0;
            masterControls.push_back(&press);
        }
    }
    // add controls that were created through Commands::Execute
    for(ControlPress* executedControl : Command::executedControls) {
        masterControls.push_back(executedControl);
    }
	
    for (Command *command : Command::allCommands) {
        // determine which commands are active and then send the controls to their execute function
		controlsToSend.clear();
        if(command->type == pros::E_CONTROLLER_MASTER) {
            for (ControlPress *control : masterControls) {
                if (Contains(command->controls, control->control)) {
                    // the command is "active" and will be executed
                   controlsToSend.push_back(control);

                }
            }
        } else {
            // partner controls
        }
        if (!controlsToSend.empty()) {
			printf("connamd! \n");
            command->Execute(controlsToSend);
        }
    }

    Command::controlsLastActive.clear();
    for (ControlPress *press : newControls) {
        Command::controlsLastActive.push_back(press->control);
    }

}

void Commands::Execute(Control control) {
    Commands::Execute(control, 0);
}

void Commands::Execute(Control control, int value) {
    Commands::Execute(control, value, PressType::PRESSED);
}

void Commands::Execute(Control control, int value, PressType pressType) {
    ControlPress press = {};
    press.pressType = pressType;
    press.control = control;
    press.value = value;
    Command::executedControls.push_back(&press);
}
