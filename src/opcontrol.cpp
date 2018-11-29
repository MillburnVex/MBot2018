
#include "../include/main.h"
#include "BotComponent.h"
#include "Command.h"

void Update() {
    Commands::Update();
    Components::Update();
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.

 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {

    pros::Controller master = pros::Controller(pros::E_CONTROLLER_MASTER);
    pros::c::controller_clear(pros::E_CONTROLLER_MASTER);
    while (!master.is_connected()) {
        pros::lcd::print(1, "Connect master controller!");
        pros::lcd::print(2, "Press button on left to skip");
        if (pros::lcd::read_buttons() == 4) {
            break;
        }
        pros::Task::delay(500);
    }
    master.rumble("..");
    pros::c::controller_print(pros::E_CONTROLLER_MASTER, 0, 0, "Lock and load lads");
    pros::lcd::clear();
	
	uint32_t time = pros::millis();
	while (true) {
		Update();
		pros::Task::delay_until(&time, 20);
	}
	
    
}
