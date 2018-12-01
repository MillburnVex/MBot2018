
#include "../include/main.h"
#include "../include/api.h"
#include "BotComponent.h"
#include "Command.h"

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */

bool running = true;

void AutonomousUpdate(void* params) {
	uint32_t time = pros::millis();
	while (running) {
		Commands::Update();
		Components::Update();
		pros::Task::delay_until(&time, 20);
	}
}



void frontauton(bool blue) {
	int bluemult = blue ? 1 : -1;

	pros::delay(200);
	Commands::Press(C_DRIVE_LINEAR_TO, 1350);
	Commands::Press(C_BALL_LIFT_UP);
	pros::delay(1800);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);

	Commands::Press(C_DRIVE_LINEAR_TO, -1100);
	pros::delay(1800);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);

	Commands::Press(C_DRIVE_ROTATE_TO, bluemult * 380);//turn to shot
	pros::delay(800);
	Commands::Release(C_DRIVE_ROTATE_TO);

	Commands::Press(C_INDEX); //shot 1
	pros::delay(500);
	Commands::Release(C_INDEX);

	Commands::Press(C_DRIVE_LINEAR_TO, 800);//2nd shot
	pros::delay(1000);
	Commands::Release(C_DRIVE_LINEAR_TO);

	Commands::Press(C_INDEX); //shot 2
	pros::delay(800);
	Commands::Release(C_INDEX);
	pros::delay(500);

	Commands::Press(C_DRIVE_ROTATE_TO, bluemult * 130);//1st bototm flag turn
	pros::delay(900);
	Commands::Release(C_DRIVE_ROTATE_TO);

	Commands::Press(C_DRIVE_LINEAR_TO, 1000);//push in
	pros::delay(1000);
	Commands::Release(C_DRIVE_LINEAR_TO);
	pros::delay(400);

	Commands::Release(C_BALL_LIFT_UP);
	pros::delay(100);


	Commands::Press(C_DRIVE_LINEAR_TO, -1100);//push out
	pros::delay(1800);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(100);

	Commands::Press(C_BALL_LIFT_DOWN);

	Commands::Press(C_DRIVE_ROTATE_TO, bluemult * -320);
	pros::delay(1200);
	Commands::Release(C_DRIVE_ROTATE_TO);

	Commands::Press(C_DRIVE_LINEAR, 80);
	pros::delay(1200);
	Commands::Release(C_DRIVE_LINEAR, 80);

	Commands::Press(C_DRIVE_LINEAR_TO, 500);
	pros::delay(1000);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);

	Commands::Release(C_BALL_LIFT_DOWN);
}

void backauton(bool blue) {
	int bluemult = blue ? 1 : -1;

	Commands::Press(C_DRIVE_LINEAR_TO, 1350);
	Commands::Press(C_BALL_LIFT_UP);
	pros::delay(1800);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);

	Commands::Press(C_DRIVE_LINEAR_TO, -1100);
	pros::delay(1800);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);

	Commands::Press(C_DRIVE_ROTATE_TO, bluemult * 380); //turn to flags
	pros::delay(800);
	Commands::Release(C_DRIVE_ROTATE_TO);
	pros::delay(200);

	Commands::Press(C_DRIVE_LINEAR_TO, 1100);//move to pos
	pros::delay(1800);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);

	Commands::Press(C_INDEX);
	pros::delay(800);
	Commands::Release(C_INDEX);
	pros::delay(500);

	Commands::Press(C_DRIVE_ROTATE_TO, bluemult * -120); //turn to flags
	pros::delay(800);
	Commands::Release(C_DRIVE_ROTATE_TO);
	pros::delay(200);

	Commands::Press(C_INDEX);
	pros::delay(800);
	Commands::Release(C_INDEX);
	pros::delay(500);

	Commands::Press(C_DRIVE_ROTATE_TO, bluemult * 120); //turn back
	pros::delay(800);
	Commands::Release(C_DRIVE_ROTATE_TO);
	pros::delay(200);


	Commands::Press(C_DRIVE_LINEAR_TO, 1100);//move back to tile
	pros::delay(1800);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);

	Commands::Press(C_DRIVE_ROTATE_TO, bluemult * -760);// turn back away
	pros::delay(800);
	Commands::Release(C_DRIVE_ROTATE_TO);

	Commands::Press(C_BALL_LIFT_DOWN); //flip cap proc

	Commands::Press(C_DRIVE_LINEAR, 30);
	pros::delay(1200);
	Commands::Release(C_DRIVE_LINEAR, 30);

	Commands::Release(C_BALL_LIFT_DOWN);
}


void autonomous() {
	pros::Task updateTask(AutonomousUpdate, (void*) "i'd dab to that",
		TASK_PRIORITY_DEFAULT + 1, TASK_STACK_DEPTH_DEFAULT, "Auton Update");

	frontauton(true);

	running = false;
}