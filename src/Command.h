#pragma once

#include <utility>
#include <vector>
#include "../include/api.h"
#include <array>
#include <vector>

typedef enum {
    PRESS_NOT_ACTIVE = -1, PRESSED = 50, REPEATED = 52, RELEASED = 53
} PressType;

#define CONTROL_NOT_ACTIVE 10000
#define ANALOG_CONTROL_ACTIVE_THRESHOLD 3

typedef enum {
	C_DRIVE_LINEAR = pros::E_CONTROLLER_ANALOG_RIGHT_Y,
	C_DRIVE_LINEAR_TO = 100,
	C_DRIVE_ROTATE = pros::E_CONTROLLER_ANALOG_LEFT_X,
	C_DRIVE_SET_ROTATION = 101,
	C_DRIVE_ROTATE_TO_ABSOLUTE = 104,
	C_FLYWHEEL_SET = 102,
	C_BALL_LIFT_UP = pros::E_CONTROLLER_DIGITAL_L1,
	C_BALL_LIFT_DOWN = pros::E_CONTROLLER_DIGITAL_L2,
	C_SHOOT = pros::E_CONTROLLER_DIGITAL_R1,
	C_AIM = 103,
	C_FLYWHEEL_SLOW = pros::E_CONTROLLER_DIGITAL_RIGHT,
	C_ARM_UP = pros::E_CONTROLLER_DIGITAL_UP,
	C_ARM_DOWN = pros::E_CONTROLLER_DIGITAL_DOWN,
	C_TEST = 109,
	C_LOAD_BALL = 105,
	C_DOUBLE_SHOT = pros::E_CONTROLLER_DIGITAL_R2
} Control;

typedef enum {
    MASTER = pros::E_CONTROLLER_MASTER,
    PARTNER = pros::E_CONTROLLER_PARTNER,
    BOTH,
    CONTOLLER_NOT_ACTIVE = -1
} Controller;

typedef struct {
    PressType pressType;
    int control;
    int value;
    Controller controller;
} ControlPress;

/**
 * An abstract class that can be extended to create commands. On instantiation of a command, it is automatically added
 * to a static list which will be checked each tick with the latest values. Commands will be executed (each tick)
 * if any of their requested controls are not inactive. The best practice is to call only non-blocking methods inside
 * of the execute method, as blocking the thread will stop other commands from executing as well
 */
class Command {
public:
    static std::vector<Command *> allCommands;
    static std::vector<std::pair<int, int>> controlsLastActive;
    // the ones that were created by calling Commands::Execute
    static std::vector<ControlPress> executedControls;

    std::vector<int> controls;
    Controller type;

/**
 * @param type one of Controller::{MASTER, PARTNER, BOTH}
 * @param controls the controls this wants to be sent
 */
    Command(Controller type, std::vector<int> controls);

/**
 * Executes the command with the given values. For analog controls, the value will be in [-127, 127]. For digital controls,
 * the value will be one of DIGITAL_{PRESSED, REPEATED, RELEASED}
 * @param values the control, control_value pairs that this command indicated it wanted to recieve in the constructor. To
 * get a control_value from the wector, just use Commands::GetValue(values, control)
 */
    virtual void Execute(std::vector<ControlPress> &values) {
    }
};

namespace Commands {

    /**
     * @return the value of the ControlPress representing the control parameter
     */
    int GetValue(std::vector<ControlPress> &values, int control);

    int GetValue(std::vector<ControlPress> &value, int control, Controller controller);

    /**
    * @return the press type of the ControlPress representing the control parameter
    */
    PressType GetPressType(std::vector<ControlPress> &values, int control);

    PressType GetPressType(std::vector<ControlPress> &values, int control, Controller controller);

    Controller GetController(std::vector<ControlPress> &values, int control);

    bool Contains(std::vector<std::pair<int, int>> &vec, int controller, int control);

    bool Contains(std::vector<int> &vec, std::vector<int> &i);

    bool Contains(std::vector<int> &vec, int i);

    bool Contains(std::vector<ControlPress *> presses, int control);

	bool Contains(std::vector<ControlPress> presses, Control control);

    void Press(Control control);

    void Press(Control control, int value);

    void Press(Control control, int value, Controller controller);

	void Release(Control control);

	void Release(Control control, int value);

	void Release(Control control, int value, Controller controller);

	void Execute(Control control, int value);

	void Execute(Control control, int value, int millisBeforeCancel);

	void Clear();

    void Update();

    void Init();
}
