#pragma once

#include <utility>
#include <vector>
#include "api.h"
#include <array>

typedef enum {
	PRESSED = 1000, REPEATED, RELEASED
} PressType;

typedef struct {
	PressType pressType;
	int control;
	int value;
} ControlPress;

#define CONTROL_NOT_ACTIVE 10000
#define ANALOG_CONTROL_ACTIVE_THRESHOLD 3

/**
 * An abstract class that can be extended to create commands. On instantiation of a command, it is automatically added
 * to a static list which will be checked each tick with the latest values. Commands will be executed (each tick)
 * if any of their requested controls are not inactive. The best practice is to call only non-blocking methods inside
 * of the execute method, as blocking the thread will stop other commands from executing as well
 */
class Command {
public:
static std::vector<Command*> allCommands;
static std::vector<int> controlsLastActive;

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
 * @param values the control, control_value pairs that this command indicated it wanted to receive in the constructor. To
 * get a control_value from the wector, just use Commands::GetValue(values, control)
 */
virtual void Execute(std::vector<ControlPress*> &values) {
}
};

namespace Commands {

	/**
	 * Returns the value of the ControlPress representing the control parameter
	 */
int GetValue(std::vector<ControlPress*>& values, int control);

 /**
 * Returns the press type of the ControlPress representing the control parameter
 */
PressType GetPressType(std::vector<ControlPress*>& values, int control);

bool Contains(std::vector<int> &vec, int i);

bool Contains(std::vector<int> &vec, std::vector<int> &i);

bool Contains(std::vector<ControlPress*> presses, int control);

void Update();

void Init();
}
