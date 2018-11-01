#include <set>
#include "Command.h"
#include "BotComponent.h"

std::set<Command *> Command::allCommands;
std::set<int> Command::controlsLastActive;
std::set<ControlPress*> Command::executedControls;

class DriveCommands : public Command {
public:
    DriveCommands() : Command(pros::E_CONTROLLER_MASTER,
                              {Control::C_DRIVE_LINEAR, Control::C_DRIVE_ROTATE}) {}

    void Execute(std::set<ControlPress *> &values) override {
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

    void Execute(std::set<ControlPress *> &values) override {
        bool up = (Commands::GetPressType(values, Control::C_BALL_LIFT_UP) == PressType::PRESSED);
        bool down = (Commands::GetPressType(values, Control::C_BALL_LIFT_DOWN) == PressType::PRESSED);
        // here's where we check sensor values
        if(!up && !down) {
            Components::Execute(ActionType::BALL_LIFT_STOP);
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

    void Execute(std::set<ControlPress *> &values) override {
        bool shoot = (Commands::GetPressType(values, Control::C_SHOOT) == PressType::PRESSED);
        if(shoot) {
            // ok bazinga we going
        }
    }
};

void Commands::Init() {
    new DriveCommands();
    new BallLiftCommands();
}

Command::Command(pros::controller_id_e_t type, std::set<int> controls) : type(type), controls(std::move(controls)) {
    Command::allCommands.insert(this);
}

int Commands::GetValue(std::set<ControlPress *> &vec, int control) {
    for (ControlPress *t : vec) {
        if (t->control == control)
            return t->value;
    }
    return CONTROL_NOT_ACTIVE;
}

PressType Commands::GetPressType(std::set<ControlPress *> &vec, int control) {
    for (ControlPress *t : vec) {
        if (t->control == control)
            return t->pressType;
    }
    throw "No control with the matching id in the std::vector<ControlPress*>";
}

bool Commands::Contains(std::set<int> &vec, int i) {
    return vec.find(i) != vec.end();
}

bool Commands::Contains(std::set<ControlPress *> presses, int control) {
    for (ControlPress *press : presses) {
        if (press->control == control) {
            return true;
        }
    }
    return false;
}

bool Commands::Contains(std::set<int> &vec, std::set<int> &i) {
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
    std::set<ControlPress *> newControls;

    std::set<ControlPress *> masterControls;

    // loop through all the values in the controller_digital_e_t enum
    for (int i = pros::E_CONTROLLER_DIGITAL_L1; i < pros::E_CONTROLLER_DIGITAL_A; i++) {
        auto button = static_cast<pros::controller_digital_e_t>(i);
        if (master.get_digital(button)) {
            ControlPress press = {};
            press.control = button;
            press.value = 1;
            // press type assigned later
            newControls.insert(&press);
        }
    }
    // loop through all the values in the controller_analog_e_t enum
    for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i < pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
        auto control = static_cast<pros::controller_analog_e_t>(i);
        int value = master.get_analog(control);
        if (value < -ANALOG_CONTROL_ACTIVE_THRESHOLD || value > ANALOG_CONTROL_ACTIVE_THRESHOLD) {
            ControlPress press = {};
            press.control = control;
            press.value = value;
            // press type assigned later
            newControls.insert(&press);
        }
    }
    // check for new presses and repeated presses
    for (ControlPress *i : newControls) {
        ControlPress press = {};
        // assign press type
        if (Contains(Command::controlsLastActive, i->control)) {
            press.pressType = PressType::REPEATED;
        } else {
            press.pressType = PressType::PRESSED;
        }
        press.control = i->control;
        press.value = i->value;
        masterControls.insert(&press);
    }
    // check for releases
    for (int i : Command::controlsLastActive) {
        if (!Contains(newControls, i)) {
            ControlPress press = {};
            press.pressType = PressType::RELEASED;
            press.control = i;
            press.value = 0;
            masterControls.insert(&press);
        }
    }
    // add controls that were created through Commands::Execute
    for(ControlPress* executedControl : Command::executedControls) {
        masterControls.insert(executedControl);
    }
    for (Command *command : Command::allCommands) {
        std::set<ControlPress *> controlsToSend;
        // determine which commands are active and then send the controls to their execute function
        if(command->type == pros::E_CONTROLLER_MASTER) {
            for (ControlPress *control : masterControls) {
                if (Contains(command->controls, control->control)) {
                    // the command is "active" and will be executed
                    controlsToSend.insert(control);
                }
            }
        } else {
            // partner controls
        }
        if (!controlsToSend.empty()) {
            command->Execute(controlsToSend);
        }
    }
    Command::controlsLastActive.clear();
    for (ControlPress *press : newControls) {
        Command::controlsLastActive.insert(press->control);
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
    Command::executedControls.insert(&press);
}
