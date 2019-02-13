#include "Component.h"
#include "Command.h"
#include "Robot.h"
#include "../include/main.h"

bool enableAutonSelector = false;

std::string GetStrategyName(Strategy strat) {
	if(strat == FRONT_PARK) {
		return "FRONT PARK";
	} else if(strat == FRONT_NO_PARK_CAP_FLIP) {
		return "FRONT NO PARK CAP FLIP";
	} else if(strat == FRONT_NO_PARK_DOUBLE_SCRAPE_MIDDLE) {
		return "FRONT NO PARK DOUBLE SCRAPE MIDDLE";
	} else if(strat == FRONT_NO_PARK_DOUBLE_SCRAPE_DESCORE) {
		return "FRONT NO PARK DOUBLE SCRAPE DESCORE";
	} else if(strat == BACK_PARK_DESCORE) {
		return "BACK PARK DESCORE";
	} else if(strat == BACK_PARK_MIDDLE) {
		return "BACK PARK MIDDLE";
	} else if(strat == BACK_PARK_TWO_CAP) {
		return "BACK PARK TWO CAP";
	} else if(strat == BACK_NO_PARK_DESCORE_CAP_FLIP) {
		return "BACK NO PARK DESCORE CAP FLIP";
	} else if(strat == BACK_NO_PARK_MIDDLE_CAP_FLIP) {
		return "BACK NO PARK MIDDLE CAP FLIP";
	} else if(strat == BACK_NO_PARK_TWO_CAP) {
		return "BACK NO PARK TWO CAP";
	}
	return "UNKNOWN STRATEGY";
}

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
		pros::lcd::print(1, "Select team: RED - Left, SKIP - Mid, BLUE - Right");
		while (buttons == 0) {
			buttons = pros::lcd::read_buttons();
			pros::delay(50);
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
			while (pros::lcd::read_buttons() != 0) {
				pros::delay(50);
			}
			Strategy strategy = FRONT_PARK;
			pros::lcd::print(1, "Select strat: CYCLE BACKWARDS - Left, SELECT - Mid, CYCLE FORWARDS - Right");

			buttons = 0;
			while (buttons == 0) {
				buttons = pros::lcd::read_buttons();
				pros::delay(50);
			}

			Robot::SetAutonStrategy(strategy);
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
	Robot::SetDriveBrakeMode(pros::E_MOTOR_BRAKE_COAST);
	Commands::Clear();
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
