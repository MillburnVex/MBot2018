#pragma once

#include <utility>
#include <vector>
#include "../include/api.h"
#include <array>
#include <vector>

typedef enum {
    NOT_ACTIVE = -1, PRESSED = 50, REPEATED = 52, RELEASED = 53
} PressType;

typedef struct {
    PressType pressType;
    int control;
    int value;
} ControlPress;

#define CONTROL_NOT_ACTIVE 10000
#define ANALOG_CONTROL_ACTIVE_THRESHOLD 3

typedef enum {
    C_DRIVE_LINEAR = pros::E_CONTROLLER_ANALOG_LEFT_Y,
    C_DRIVE_ROTATE = pros::E_CONTROLLER_ANALOG_LEFT_X,
    C_BALL_LIFT_UP = pros::E_CONTROLLER_DIGITAL_L1,
    C_BALL_LIFT_DOWN = pros::E_CONTROLLER_DIGITAL_L2,
    C_CAP_LIFT_UP = -1,
    C_CAP_LIFT_DOWN = -1,
    C_SHOOT = pros::E_CONTROLLER_DIGITAL_X,
	C_AIM = pros::E_CONTROLLER_DIGITAL_B,
	C_CLAW_FOLD_UP = pros::E_CONTROLLER_DIGITAL_R1,
	C_CLAW_FOLD_DOWN = pros::E_CONTROLLER_DIGITAL_R2,
    C_CLAW_ROTATE_180 = pros::E_CONTROLLER_DIGITAL_UP,
} Control;

/**
 * An abstract class that can be extended to create commands. On instantiation of a command, it is automatically added
 * to a static list which will be checked each tick with the latest values. Commands will be executed (each tick)
 * if any of their requested controls are not inactive. The best practice is to call only non-blocking methods inside
 * of the execute method, as blocking the thread will stop other commands from executing as well
 */
class Command {
public:
    static std::vector<Command *> allCommands;
    static std::vector<int> controlsLastActive;
    // the ones that were created by calling Commands::Execute
    static std::vector<ControlPress*> executedControls;

    std::vector<int> controls;
    pros::controller_id_e_t type;

/**
 * @param type one of pros::E_CONTROLLER_{MASTER, PARTNER}
 * @param controls the controls this wants to be sent
 */
    Command(pros::controller_id_e_t type, std::vector<int> controls);

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

    /**
    * @return the press type of the ControlPress representing the control parameter
    */
    PressType GetPressType(std::vector<ControlPress> &values, int control);

    bool Contains(std::vector<int> &vec, int i);

    bool Contains(std::vector<int> &vec, std::vector<int> &i);

    bool Contains(std::vector<ControlPress *> presses, int control);

    void Execute(Control control);

    void Execute(Control control, int value);

    void Execute(Control control, int value, PressType pressType);

    void Update();

    void Init();
}
