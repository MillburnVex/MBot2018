#include <vector>
#include <algorithm>
#include "Command.h"
#include "Robot.h"
#include "Component.h"
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
            Robot::GetMasterController().rumble("...");
        }
    }
};

class DriveCommands : public Command {
public:
    DriveCommands() : Command(Controller::MASTER,
                              {Control::C_DRIVE_LINEAR, Control::C_DRIVE_ROTATE, Control::C_DRIVE_LINEAR_TO,
                               Control::C_DRIVE_ROTATE_TO, Control::C_AIM}) {}

    void Execute(std::vector<ControlPress> &values) override {
		bool linearReleased = Commands::GetPressType(values, Control::C_DRIVE_LINEAR) == RELEASED;
		bool rotateReleased = Commands::GetPressType(values, Control::C_DRIVE_ROTATE) == RELEASED;
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
                Components::Execute(ActionType::LINEAR_TO, linearTo);
            } else if(linearReleased) {
                Components::Execute(ActionType::DRIVE_LINEAR, 0);
            }
            if (rotation != CONTROL_NOT_ACTIVE) {
                rotation = (rotation > 0 ? 127.0 : -127.0) * pow(std::abs(rotation) / 127.0, 17.0 / 7.0);
                Components::Execute(ActionType::DRIVE_ROTATE, rotation);
            } else if (rotateTo != CONTROL_NOT_ACTIVE) {
                Components::Execute(ActionType::ROTATE_TO, rotateTo);
            } else if(rotateReleased) {
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
        if (Robot::IsInManualMode()) {
            if (up) {
                pos += 10;
            } else if (down) {
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
                               {Control::C_BALL_LIFT_DOWN, Control::C_BALL_LIFT_UP, Control::C_LOAD_BALL}) {}

    void Execute(std::vector<ControlPress> &values) override {
        bool up = (Commands::GetPressType(values, Control::C_BALL_LIFT_UP) != PressType::PRESS_NOT_ACTIVE);
        bool down = (Commands::GetPressType(values, Control::C_BALL_LIFT_DOWN) != PressType::PRESS_NOT_ACTIVE);
		bool loadBall = (Commands::GetPressType(values, Control::C_LOAD_BALL) != PressType::PRESS_NOT_ACTIVE);
		if (loadBall) {
			if (Robot::BallLoaded()) {
				Commands::Release(C_LOAD_BALL);
			}
			else {
				up = true;
			}
		}
		if (!up && !down) {
            Components::Execute(ActionType::REAPER_RUN, -25);
        } else if (up) {
            Components::Execute(ActionType::REAPER_RUN, -127);
        } else {
            Components::Execute(ActionType::REAPER_RUN, 127);
        }
    }
};

class FlywheelCommand : public Command {
public:
    FlywheelCommand() : Command(Controller::BOTH, {
            Control::C_FLYWHEEL_SET, Control::C_FLYWHEEL_SLOW
    }) {}

    bool slowMode = false;

    void Execute(std::vector<ControlPress> &values) override {
        if ((Commands::GetPressType(values, Control::C_FLYWHEEL_SET) == PressType::PRESSED)) {
            Components::Execute(ActionType::FLYWHEEL_RUN, Commands::GetValue(values, Control::C_FLYWHEEL_SET));
        } else if (Commands::GetPressType(values, Control::C_FLYWHEEL_SLOW) == PressType::PRESSED) {
            slowMode = !slowMode;
            Robot::GetMasterController().rumble(".");
            Robot::GetMasterController().print(0, 1,
                                               (slowMode) ? std::string("532").c_str() : std::string("600").c_str());
        }
        Components::Execute(ActionType::FLYWHEEL_RUN, (slowMode) ? 532 : 600);
    }
};

class VisionCommand : public Command {
public:
    const int NUM_VISION_OBJECTS = 4;

    VisionCommand() : Command(Controller::BOTH, {
            Control::C_AIM
    }) {}

    void Execute(std::vector<ControlPress> &values) override {
        bool aim = (Commands::GetPressType(values, Control::C_AIM) != PressType::PRESS_NOT_ACTIVE);

        pros::vision_object_s_t objects[NUM_VISION_OBJECTS];
        int team = Robot::GetTeam() == RED ? 1 : 0;
        std::int32_t objcount = Robot::GetCamera().read_by_sig(0, NUM_VISION_OBJECTS, team, objects);
        if (objcount > NUM_VISION_OBJECTS) {
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
    }
};

class DoubleShotCommand : public Command {
public:
	int ticksSinceStart = -1;

	const int TICKS_TILL_FINISH = 500;

	const int TICKS_TILL_FIRST_SHOT = 0;
	const int TICKS_TILL_SECOND_SHOT = 60;

	const int DRIVE_SPEED = 127;

	DoubleShotCommand() : Command(Controller::BOTH, {
		Control::C_DOUBLE_SHOT
		}) {}

	void Execute(std::vector<ControlPress> &values) override {
		bool wantsToShoot = Commands::GetPressType(values, C_DOUBLE_SHOT) == PRESSED;
		bool wantsToCancel = Commands::GetPressType(values, C_DOUBLE_SHOT) == RELEASED;
		if (wantsToCancel) {
			Commands::Release(C_BALL_LIFT_UP);
			ticksSinceStart = -1;
		}
		else if(wantsToShoot){
			ticksSinceStart = 0;
			Commands::Press(C_BALL_LIFT_UP);
		}

		if (ticksSinceStart != -1) {
			// in the process of shooting
			Components::Execute(ActionType::DRIVE_LINEAR, DRIVE_SPEED);
			if (std::abs(ticksSinceStart - TICKS_TILL_SECOND_SHOT) < 15) {
				Components::Execute(ActionType::INDEXER_RUN, -70);
			}
			else {
				Components::Execute(ActionType::INDEXER_RUN, -127);
			}
			if (ticksSinceStart == TICKS_TILL_FINISH) {
				ticksSinceStart = -1;
				Commands::Release(C_BALL_LIFT_UP);
				return;
			}
			ticksSinceStart++;
		}
	}
};

class ShootCommand : public Command {
public:

    bool shooting = false;
    // used to determine if the ball has been actually shot
    bool ballGoneThroughSecondZoneWhileShooting = false;

    int ticksHeldDown = -1;

    const int GUARANTEED_SHOOT_TICKS = 50;

	const int GUARANTEED_INTAKE_TICKS = 50;

    ShootCommand() : Command(Controller::BOTH, {
            Control::C_SHOOT, Control::C_AIM, Control::C_FLYWHEEL_SET, Control::C_FLYWHEEL_SLOW, Control::C_DOUBLE_SHOT
    }) {}

    void Execute(std::vector<ControlPress> &values) override {

        bool firstZone = Robot::BallInFirstZone();
        bool secondZone = Robot::BallInSecondZone();

        if(Commands::GetPressType(values, Control::C_SHOOT) == PressType::PRESSED) {
            shooting = true;
            ticksHeldDown = 0;
		}
		else if (Commands::GetPressType(values, Control::C_SHOOT) == PressType::REPEATED) {
            ticksHeldDown++;
        } else if(Commands::GetPressType(values, Control::C_SHOOT) == PressType::PRESS_NOT_ACTIVE) {
			if (ticksHeldDown != -1) {
			// released but waiting to finish
				if (ticksHeldDown > GUARANTEED_INTAKE_TICKS) {
					ticksHeldDown = -1;
					return;
				}
				else {
					ticksHeldDown++;
				}
				// continue counting
				if (ticksHeldDown > GUARANTEED_SHOOT_TICKS) {
					shooting = false;
				}
			}
        }
        if(shooting && secondZone) {
             ballGoneThroughSecondZoneWhileShooting = true;
        }

		if (shooting || (ticksHeldDown != -1 && ticksHeldDown < GUARANTEED_INTAKE_TICKS)) {
			Components::Execute(ActionType::REAPER_RUN, -127);
		}

        if (Robot::IsInManualMode()) {
            if ((Commands::GetPressType(values, Control::C_SHOOT) != PressType::PRESS_NOT_ACTIVE)) {
                Robot::GetMotor(MotorID::INDEXER)->SetVoltage(-100);
            } else {
                Robot::GetMotor(MotorID::INDEXER)->SetVoltage(0);
            }
        } else if(Commands::GetPressType(values, Control::C_DOUBLE_SHOT) == PRESS_NOT_ACTIVE) {
            if(shooting) {
                if(!secondZone && ballGoneThroughSecondZoneWhileShooting) {
                    // the ball has totally passed the second zone, meaning it has been shot
                    shooting = false;
                    ballGoneThroughSecondZoneWhileShooting = false;
                    Commands::Release(C_SHOOT);
                    Robot::GetMasterController().rumble(".");
                    Components::Execute(ActionType::INDEXER_RUN, 0);
                } else {
                    // in the process of firing
                    Components::Execute(ActionType::INDEXER_RUN, -127);
                }
            } else {
                // if there is no ball
                if(!firstZone && !secondZone) {
                    // preload
                    Components::Execute(ActionType::INDEXER_RUN, -80);
                } else {
                    // ball is too far in
                    if(secondZone) {
                        Components::Execute(ActionType::INDEXER_RUN, 40);
                    } else {
                        // just right
                        Components::Execute(ActionType::INDEXER_RUN, 0);
                    }
                }
            }
        }
    }

};

void Commands::Init() {
    new DriveCommands();
    new ShootCommand();
    new ReaperCommands();
	new DoubleShotCommand();
    new FlywheelCommand();
    new VisionCommand();
    new ArmCommands();
    new TestCommand();
}

Command::Command(Controller type, std::vector<int> controls) : type(type), controls(std::move(controls)) {
    Command::allCommands.push_back(this);
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
    commandMutex.take(1000);
    ControlPress press = {};
    press.controller = controller;
    press.pressType = PressType::PRESSED;
    press.control = control;
    press.value = value;
    Command::executedControls.push_back(press);
    commandMutex.give();
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

void Commands::Execute(Control control, int value) {
    Commands::Execute(control, value, -1);
}

void Commands::Execute(Control control, int value, int millisBeforeCancel) {
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

void Commands::Clear() {
    Command::controlsLastActive.clear();
    Command::executedControls.clear();
}