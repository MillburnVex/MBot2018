
#include "../include/main.h"
#include "BotComponent.h"
#include "Command.h"


pros::ADIUltrasonic sonic(1, 2);
pros::Controller master = pros::Controller(pros::E_CONTROLLER_MASTER);

int timer = 0;
void Update() {
    Commands::Update();
    Components::Update();

	if(sonic.get_value() > 2 && sonic.get_value() < 100)
	{
		if(timer == -1)
		{
			if(timer < 20)
			{
				timer++;
				master.rumble("..");
			}
		}
	}else{
		timer = -1;
	}
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
    pros::c::controller_clear(pros::E_CONTROLLER_MASTER);
    while (!master.is_connected()) {
        pros::lcd::print(1, "Connect master controller!");
        pros::lcd::print(2, "Press button on left to skip");
        if (pros::lcd::read_buttons() == 4) {
            break;
        }
        pros::Task::delay(500);
    }
    pros::lcd::clear();
	
	uint32_t time = pros::millis();
	while (true) {
		Update();
		pros::Task::delay_until(&time, 20);
	}
	
    
}
