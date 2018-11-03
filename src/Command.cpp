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

    void Execute(std::vector<ControlPress> &values) override {
        int linear = Commands::GetValue(values, Control::C_DRIVE_LINEAR);
        int rotation = Commands::GetValue(values, Control::C_DRIVE_ROTATE);
		if(linear != CONTROL_NOT_ACTIVE)
		{
			Components::Execute(ActionType::DRIVE_LINEAR, linear * (1.57));
		}else
		{
			Components::Execute(ActionType::DRIVE_LINEAR, 0);
		}
        if(rotation != CONTROL_NOT_ACTIVE)
        {
			Components::Execute(ActionType::DRIVE_ROTATE, rotation * (1.57));
        }else
        {
			Components::Execute(ActionType::DRIVE_ROTATE, 0);
        }

        
    }
};

class BallLiftCommands : public Command {
public:
    BallLiftCommands() : Command(pros::E_CONTROLLER_MASTER,
                                 {Control::C_BALL_LIFT_DOWN, Control::C_BALL_LIFT_UP}) {}

    void Execute(std::vector<ControlPress> &values) override {
        bool up = (Commands::GetPressType(values, Control::C_BALL_LIFT_UP) != PressType::NOT_ACTIVE);
        bool down = (Commands::GetPressType(values, Control::C_BALL_LIFT_DOWN) != PressType::NOT_ACTIVE);
        // here's where we check sensor values
        if(!up && !down) {
            Components::Execute(ActionType::BALL_LIFT_RUN, 0);
        } else if(up) {
            Components::Execute(ActionType::BALL_LIFT_RUN, -100);
        } else {
            Components::Execute(ActionType::BALL_LIFT_RUN, 100);
        }
    }
};

class ShootCommand : public Command {
public:
    ShootCommand() : Command(pros::E_CONTROLLER_MASTER, {
            Control::C_SHOOT
    }) {}

    void Execute(std::vector<ControlPress> &values) override {
        bool shoot = (Commands::GetPressType(values, Control::C_SHOOT) != PressType::NOT_ACTIVE);
		//vex::vision::signature BLUE_CLOSE (1, -3681, -3031, -3356, 13071, 14649, 13860, 8.4, 0);
		//vex::vision::signature RED_CLOSE(2, 10493, 11317, 10905, -665, -323, -494, 8.3, 0);
        if(shoot) {
			Components::Execute(ActionType::FLYWHEEL_RUN, -120);
        }
    	else
        {
			Components::Execute(ActionType::FLYWHEEL_RUN, -100);
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

int Commands::GetValue(std::vector<ControlPress> &vec, int control) {
    for (ControlPress t : vec) {
        if (t.control == control)
            return t.value;
    }
    return CONTROL_NOT_ACTIVE;
}

PressType Commands::GetPressType(std::vector<ControlPress> &vec, int control) {
    for (ControlPress t : vec) {
        if (t.control == control)
            return t.pressType;
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
    std::vector<ControlPress> newControls;

    std::vector<ControlPress> masterControls;
	std::vector<ControlPress> controlsToSend;

    // loop through all the values in the controller_digital_e_t enum
    for (int i = pros::E_CONTROLLER_DIGITAL_L1; i <= pros::E_CONTROLLER_DIGITAL_A; i++) {
        auto button = static_cast<pros::controller_digital_e_t>(i);
		ControlPress press = {};
		press.control = button;
		press.value = 1;
        if (master.get_digital(button)) {
			if (Contains(Command::controlsLastActive, press.control)) {
				// was active and still is
				press.pressType = PressType::REPEATED;
			}
			else {
				// wasn't active
				press.pressType = PressType::PRESSED;
			}
			newControls.push_back(press);
			masterControls.push_back(press);
        } else {
	        // not active
			if(Contains(Command::controlsLastActive, press.control)) {
				press.pressType = PressType::RELEASED;
				masterControls.push_back(press);
				// don't add to new controls because it isn't pressed
			}
        }
    }
    // loop through all the values in the controller_analog_e_t enum
    for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i <= pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
        auto control = static_cast<pros::controller_analog_e_t>(i);
        int value = master.get_analog(control);
		ControlPress press = {};
		press.control = control;
		press.value = value;
        if (value < -ANALOG_CONTROL_ACTIVE_THRESHOLD || value > ANALOG_CONTROL_ACTIVE_THRESHOLD) {
			if (Contains(Command::controlsLastActive, press.control)) {
				// was active and still is
				press.pressType = PressType::REPEATED;
			} else {
				// wasn't active
				press.pressType = PressType::PRESSED;
			}
			newControls.push_back(press);
			masterControls.push_back(press);
        } else {
	        // this control isn't active
			// if this control previously was active:
			if(Contains(Command::controlsLastActive, i)) {
				// was active, now isn't
				press.pressType = PressType::RELEASED;
				// dont add to new controls
				masterControls.push_back(press);
			}
        }
    }
    // add controls that were created through Commands::Execute
    for(ControlPress* executedControl : Command::executedControls) {
        masterControls.push_back(*executedControl);
    }
	
    for (Command *command : Command::allCommands) {
        // determine which commands are active and then send the controls to their execute function
		controlsToSend.clear();
        if(command->type == pros::E_CONTROLLER_MASTER) {
            for (ControlPress control : masterControls) {
                if (Contains(command->controls, control.control)) {
                    // the command is "active" and will be executed
                   controlsToSend.push_back(control);
                }
            }
        } else {
            // partner controls
        }
        command->Execute(controlsToSend);
    }

    Command::controlsLastActive.clear();
    for (ControlPress press : newControls) {
        Command::controlsLastActive.push_back(press.control);
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
