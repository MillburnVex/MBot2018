#include <vector>
#include <algorithm>
#include "Command.h"
#include "Robot.h"
#include "BotComponent.h"
#include "../include/main.h"

std::vector<Command *> Command::allCommands;
std::vector<std::pair<int, int>> Command::controlsLastActive;
std::vector<ControlPress> Command::executedControls;
pros::Mutex commandMutex;

class TestCommand : public Command {
public:
    TestCommand() : Command(Controller::BOTH, {Control::C_TEST}) {}

    void Execute(std::vector<ControlPress> &values) override {
        if (Commands::GetPressType(values, Control::C_TEST) == PressType::PRESSED) {
            pros::Controller master = pros::Controller(pros::controller_id_e_t::E_CONTROLLER_MASTER);
            master.rumble("...");
        }
    }
};

class DriveCommands : public Command {
public:
    DriveCommands() : Command(Controller::MASTER,
                              {Control::C_DRIVE_LINEAR, Control::C_DRIVE_ROTATE, Control::C_DRIVE_LINEAR_TO,
                               Control::C_DRIVE_ROTATE_TO, Control::C_AIM}) {}

    void Execute(std::vector<ControlPress> &values) override {

        int linear = Commands::GetValue(values, Control::C_DRIVE_LINEAR);
        int rotation = Commands::GetValue(values, Control::C_DRIVE_ROTATE);
        int linearTo = Commands::GetValue(values, Control::C_DRIVE_LINEAR_TO);
        int rotateTo = Commands::GetValue(values, Control::C_DRIVE_ROTATE_TO);
        bool aim = (Commands::GetPressType(values, Control::C_AIM) != PressType::PRESS_NOT_ACTIVE);
        if (aim) {
            Components::Execute(ActionType::DRIVE_LINEAR, 0);
        } else {
            if (linear != CONTROL_NOT_ACTIVE) {
                linear = (linear > 0 ? 127.0 : -127.0) * pow(std::abs(linear) / 127.0, 11.0 / 7.0);
                Components::Execute(ActionType::DRIVE_LINEAR, linear);
            } else if (linearTo != CONTROL_NOT_ACTIVE) {
                Components::Execute(ActionType::DRIVE_TO, linearTo);
            } else {
                Components::Execute(ActionType::DRIVE_LINEAR, 0);
            }
            if (rotation != CONTROL_NOT_ACTIVE) {
                rotation = (rotation > 0 ? 127.0 : -127.0) * pow(std::abs(rotation) / 127.0, 17.0 / 7.0);
                Components::Execute(ActionType::DRIVE_ROTATE, rotation);
            } else if (rotateTo != CONTROL_NOT_ACTIVE) {
                Components::Execute(ActionType::ROTATE_TO, rotateTo);
            } else {
                Components::Execute(ActionType::DRIVE_ROTATE, 0);

            }
        }
    }

};

class ArmCommands : public Command {
    const int HOLDANGLE = 300;
    const int SCOREANGLE = 540;
    const int DOWNANGLE = 0;
    int pos = DOWNANGLE;
public:
    ArmCommands() : Command(Controller::BOTH,
                            {Control::C_ARM_UP, Control::C_ARM_DOWN}) {}

    void Execute(std::vector<ControlPress> &values) override {
        bool up = (Commands::GetPressType(values, Control::C_ARM_UP) == PressType::PRESSED);
        bool down = (Commands::GetPressType(values, Control::C_ARM_DOWN) != PressType::PRESS_NOT_ACTIVE);
        // here's where we check sensor values
        if(Robot::IsInManualMode()) {
            if(up) {
                pos += 10;
            } else if(down) {
                pos -= 10;
            }
        } else {
            if (down) {
                pos = DOWNANGLE;
            } else if (up) {
                if (pos == HOLDANGLE) {
                    pos = SCOREANGLE;
                } else {
                    pos = HOLDANGLE;
                }
            }
        }
        Components::Execute(ActionType::ARM_SET, pos);
    }
};

class ReaperCommands : public Command {
public:
    ReaperCommands() : Command(Controller::BOTH,
                                 {Control::C_BALL_LIFT_DOWN, Control::C_BALL_LIFT_UP}) {}

    void Execute(std::vector<ControlPress> &values) override {
        bool up = (Commands::GetPressType(values, Control::C_BALL_LIFT_UP) != PressType::PRESS_NOT_ACTIVE);
        bool down = (Commands::GetPressType(values, Control::C_BALL_LIFT_DOWN) != PressType::PRESS_NOT_ACTIVE);
        if (!up && !down) {
            Components::Execute(ActionType::REAPER_RUN, -25);
        } else if (up) {
            Components::Execute(ActionType::REAPER_RUN, -127);
        } else {
            Components::Execute(ActionType::REAPER_RUN, 127);
        }
    }
};

class ShootCommand : public Command {
public:
    const int NUM_VISION_OBJECTS = 4;

    bool shooting = false;
    bool hadBallOnInitialPress = false;

    ShootCommand() : Command(Controller::BOTH, {
            Control::C_SHOOT, Control::C_AIM, Control::C_SPEED_SET, Control::C_SLOW
    }) {}

	void Execute(std::vector<ControlPress> &values) override {
		if ((Commands::GetPressType(values, Control::C_SPEED_SET) == PressType::PRESSED)){
			Components::Execute(ActionType::FLYWHEEL_RUN, Commands::GetValue(values, Control::C_SPEED_SET));
		}
		else if (Commands::GetPressType(values, Control::C_SLOW) == PressType::PRESSED) {
			Components::Execute(ActionType::FLYWHEEL_RUN, 105);
		}else {
			Components::Execute(ActionType::FLYWHEEL_RUN, 127);
		}

        bool aim = (Commands::GetPressType(values, Control::C_AIM) != PressType::PRESS_NOT_ACTIVE);
        bool ballLoaded = Robot::GetSensor(SensorID::INDEXER_BUTTON)->GetValue() < 2300; 
		printf("E %d\n", Robot::GetSensor(SensorID::INDEXER_BUTTON)->GetValue());

        if((Commands::GetPressType(values, Control::C_SHOOT) == PressType::PRESSED)) {
            shooting = true;
            hadBallOnInitialPress = ballLoaded;
        } else if((Commands::GetPressType(values, Control::C_SHOOT) == PressType::RELEASED)) {
            shooting = false;
        }

        pros::vision_object_s_t objects[NUM_VISION_OBJECTS];
        int team = Robot::GetTeam() == RED ? 1 : 0;
        std::int32_t objcount = Robot::GetCamera().read_by_sig(0, NUM_VISION_OBJECTS, team, objects);
        if (objcount > NUM_VISION_OBJECTS)  {
            objcount = 0;
        }
        int id = -1;
        int val = 10000;
        for (int i = 0; i < objcount; i++) {
            if (val > std::abs(objects[i].x_middle_coord)) {
                id = i;
                val = std::abs(objects[i].x_middle_coord);
            }
        }

        if (aim && id != -1) {
            Components::Execute(ActionType::DRIVE_ROTATE, objects[id].x_middle_coord);
        }

        if(Robot::IsInManualMode()) {
            if((Commands::GetPressType(values, Control::C_SHOOT) != PressType::PRESS_NOT_ACTIVE)) {
                Robot::GetMotor(BotMotorID::INDEXER)->SetVoltage(-100);
            } else {
                Robot::GetMotor(BotMotorID::INDEXER)->SetVoltage(0);
            }
        } else {
            // if the user has pressed the shoot button and a ball has not been shot yet
            if (shooting) {
                if(ballLoaded) {
                    // if a ball is loaded
                    Components::Execute(ActionType::INDEXER_RUN, -127);
                    // run the indexer until the ball is gone
                } else {
                    // there is no ball loaded. this could be because there never was one, or the ball was shot
                    // check if there initially was a ball
                    if(hadBallOnInitialPress) {
                        // the ball is gone now, so you have shot once
                        pros::Controller master = pros::Controller(pros::controller_id_e_t::E_CONTROLLER_MASTER);
                        // get that sweet feedback
                        master.rumble(".");
                        shooting = false;
                        // the indexer will now behave as if the shooting button isn't being pressed
                        // this means it will continuously intake at a slower speed until it is loaded again, then it will stop
						Commands::Release(Control::C_SHOOT);
                    } else {
                        // no ball was loaded already, but the user still wants to fire. At this point we're just going to
                        // blindly obey their wishes and run the indexer
                        Components::Execute(ActionType::INDEXER_RUN, -127);
                        // this will happen until a ball is loaded and fired, or they release the shooting button
                    }
                }
            }
            if(!shooting) {
                if(ballLoaded) {
                    // if a ball is loaded, don't let it go anywhere. The motor is set to actively brake when power is set to
                    // 0, so it won't coast into the flywheel
                    Components::Execute(ActionType::INDEXER_RUN, 0);
                } else {
                    //prepare the next ball (this will run until a ball is in the indexer)
                    Components::Execute(ActionType::INDEXER_RUN, -40);
                }
            }
        }
    }

};

void Commands::Init() {
    new DriveCommands();
    new ReaperCommands();
    new ShootCommand();
    new ArmCommands();
    new TestCommand();
}

Command::Command(Controller type, std::vector<int> controls) : type(type), controls(std::move(controls)) {
    Command::allCommands.push_back(this);
}

void Commands::ExecuteUntilFinished(Control control, int value) {
	Commands::ExecuteUntilFinished(control, value, -1);
}

void Commands::ExecuteUntilFinished(Control control, int value, int millisBeforeCancel) {
	Commands::Press(control, value);
	int millisTaken = 0;
	while (Commands::Contains(Command::executedControls, control)) {
		pros::delay(Robot::GetUpdateMillis());
		millisTaken += Robot::GetUpdateMillis();
		if (millisBeforeCancel >= 0 && millisTaken >= millisBeforeCancel) {
			Commands::Release(control);
			return;
		}
	}
}

int Commands::GetValue(std::vector<ControlPress> &vec, int control) {
    int masterValue = Commands::GetValue(vec, control, Controller::MASTER);
    if (masterValue != CONTROL_NOT_ACTIVE) {
        return masterValue;
    } else {
        return Commands::GetValue(vec, control, Controller::PARTNER);
    }
}

int Commands::GetValue(std::vector<ControlPress> &vec, int control, Controller controller) {
    for (ControlPress t : vec) {
        if (t.control == control && t.controller == controller)
            return t.value;
    }
    return CONTROL_NOT_ACTIVE;
}

PressType Commands::GetPressType(std::vector<ControlPress> &vec, int control) {
    PressType masterPressType = Commands::GetPressType(vec, control, Controller::MASTER);
    if (masterPressType != PressType::PRESS_NOT_ACTIVE) {
        return masterPressType;
    } else {
        return Commands::GetPressType(vec, control, Controller::PARTNER);
    }
}

PressType Commands::GetPressType(std::vector<ControlPress> &vec, int control, Controller controller) {
    for (ControlPress t : vec) {
        if (t.control == control && t.controller == controller)
            return t.pressType;
    }
    return PressType::PRESS_NOT_ACTIVE;
}

Controller Commands::GetController(std::vector<ControlPress> &vec, int control) {
    Controller lastController = Controller::CONTOLLER_NOT_ACTIVE;
    for (ControlPress t : vec) {
        if (t.control == control) {
            if (lastController != t.controller) {
                return Controller::BOTH;
            } else {
                lastController = t.controller;
            }
        }
    }
    return lastController;
}

bool Commands::Contains(std::vector<std::pair<int, int>> &vec, int controller, int control) {
    for (std::pair<int, int> controllerAndControl : vec) {
        if (controllerAndControl.first == controller && controllerAndControl.second == control) {
            return true;
        }
    }
    return false;
}

bool Commands::Contains(std::vector<ControlPress> presses, Control control) {
	for (ControlPress press : presses) {
		if (static_cast<Control>(press.control) == control) {
			return true;
		}
	}
	return false;
}

bool Commands::Contains(std::vector<ControlPress *> presses, int control) {
    for (ControlPress *press : presses) {
        if (press->control == control) {
            return true;
        }
    }
    return false;
}

bool Commands::Contains(std::vector<int> &vec, int i) {
    return std::find(vec.begin(), vec.end(), i) != vec.end();
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

    // controls that haven't been released
    std::vector<ControlPress> newControls;
    // controls that could possibly be sent
    std::vector<ControlPress> controlPresses;
    // controls that are getting sent
    std::vector<ControlPress> controlsToSend;

    std::vector<int> controlsToRemove;

    for (int controllerID = pros::E_CONTROLLER_MASTER; controllerID <= pros::E_CONTROLLER_PARTNER; controllerID++) {

        pros::Controller controller(static_cast<pros::controller_id_e_t>(controllerID));

        // loop through all the values in the controller_digital_e_t enum
        for (int i = pros::E_CONTROLLER_DIGITAL_L1; i <= pros::E_CONTROLLER_DIGITAL_A; i++) {
            auto button = static_cast<pros::controller_digital_e_t>(i);
            ControlPress press = {};
            press.controller = static_cast<Controller>(controllerID);
            press.control = button;
            press.value = 1;
            if (controller.get_digital(button)) {
                if (Contains(Command::controlsLastActive, controllerID, i)) {
                    // was active and still is
                    press.pressType = PressType::REPEATED;
                } else {
                    // wasn't active
                    press.pressType = PressType::PRESSED;
                }
                newControls.push_back(press);
                controlPresses.push_back(press);
            } else {
                // not active
                if (Contains(Command::controlsLastActive, controllerID, i)) {
                    press.pressType = PressType::RELEASED;
                    controlPresses.push_back(press);
                    // don't add to new controls because it isn't pressed
                }
            }
        }
        // loop through all the values in the controller_analog_e_t enum
        for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i <= pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
            auto control = static_cast<pros::controller_analog_e_t>(i);
            int value = controller.get_analog(control);
            ControlPress press = {};
            press.controller = static_cast<Controller>(controllerID);
            press.control = control;
            press.value = value;
            if (value < -ANALOG_CONTROL_ACTIVE_THRESHOLD || value > ANALOG_CONTROL_ACTIVE_THRESHOLD) {
                if (Contains(Command::controlsLastActive, controllerID, i)) {
                    // was active and still is
                    press.pressType = PressType::REPEATED;
                } else {
                    // wasn't active
                    press.pressType = PressType::PRESSED;
                }
                newControls.push_back(press);
                controlPresses.push_back(press);
            } else {
                // this control isn't active
                // if this control previously was active
                if (Contains(Command::controlsLastActive, controllerID, i)) {
                    // was active, now isn't
                    press.pressType = PressType::RELEASED;
                    // dont add to new controls
                    controlPresses.push_back(press);
                }
            }
        }
    }


    // add controls that were created through Commands::Execute
    commandMutex.take(1000);
    for (int i = 0; i < Command::executedControls.size();) {
        ControlPress executedControl = Command::executedControls.at(i);
        controlPresses.push_back(executedControl);
        //printf("COLOR_HONEYDEW %d\n", executedControl.pressType);
        if (executedControl.pressType == PressType::PRESSED) {
            executedControl.pressType = PressType::REPEATED;
            Command::executedControls.at(i) = executedControl;
        }

        if (executedControl.pressType == PressType::RELEASED) {
            Command::executedControls.erase(Command::executedControls.begin() + i);

        } else
            ++i;

    }
    commandMutex.give();

    for (Command *command : Command::allCommands) {
        // determine which commands are active and then send the controls to their execute function
        controlsToSend.clear();
        for (ControlPress control : controlPresses) {
            if (Contains(command->controls, control.control) &&
                (command->type == Controller::BOTH || command->type == control.controller)) {
                // the control matches the required controls and the controller type
                controlsToSend.push_back(control);
            }
        }
        command->Execute(controlsToSend);
    }

    Command::controlsLastActive.clear();
    for (ControlPress press : newControls) {
        Command::controlsLastActive.push_back(std::pair<int, int>(press.controller, press.control));
    }
}

void Commands::Press(Control control) {
    Commands::Press(control, 0);
}

void Commands::Press(Control control, int value) {
    Commands::Press(control, value, Controller::MASTER);
}

void Commands::Press(Control control, int value, Controller controller) {
    Commands::Execute(control, value, controller, PressType::PRESSED);
}

void Commands::Release(Control control) {
    Commands::Release(control, 0);
}

void Commands::Release(Control control, int value) {
    Commands::Release(control, value, Controller::MASTER);
}

void Commands::Release(Control control, int value, Controller controller) {
    commandMutex.take(1000);
    std::vector<int> controlsToRemove;
    int i = 0;
    for (int i = 0; i < Command::executedControls.size(); i++) {
        ControlPress command = Command::executedControls.at(i);
        if (command.control == control && command.controller == controller) {
            command.pressType = RELEASED;
            Command::executedControls.at(i) = command;
        }
    }

    commandMutex.give();
}

void Commands::Execute(Control control, int value, Controller controller, PressType pressType) {
    commandMutex.take(1000);
    ControlPress press = {};
    press.controller = controller;
    press.pressType = pressType;
    press.control = control;
    press.value = value;
    Command::executedControls.push_back(press);
    commandMutex.give();
}

void Commands::Clear() {
    Command::controlsLastActive.clear();
    Command::executedControls.clear();
}