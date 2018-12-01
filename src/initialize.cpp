#include "BotComponent.h"
#include "Command.h"
#include "Robot.h"
#include "../include/main.h"

bool enableAutonSelector = true;

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
    pros::lcd::initialize();
	if (enableAutonSelector) {
		int buttons = 0;
		while (buttons != 4 || buttons != 1 || buttons != 2) {
			pros::lcd::print(1, "Select team: Blue - Right, Red - Left, Skip - Mid");
			buttons = pros::lcd::read_buttons();
		}
		if (buttons != 2) {
			Team team = RED;
			if (buttons == 4) {
				// left - red
				team = RED;
			}
			else if (buttons == 1) {
				// right - blue
				team = BLUE;
			}
			Position pos = FRONT;
			while (buttons != 4 || buttons != 1) {
				pros::lcd::print(1, "Select pos: Front - Right, Back - Left");
				buttons = pros::lcd::read_buttons();
			}
			if (buttons == 4) {
				pos = BACK;
			}
			else if (buttons == 1) {
				pos = FRONT;
			}
			Robot::SetAutonPosition(pos);
			Robot::SetTeam(team);
		}
	}
	pros::lcd::clear();
    Robot::Init();
    Components::Init();
    Commands::Init();
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
    // print("javier coindreau")
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}
