
#include "../include/main.h"
#include "../include/api.h"
#include "BotComponent.h"
#include "Command.h"
#include "Robot.h"

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
		pros::Task::delay_until(&time, Robot::GetUpdateMillis());
	}
}



void frontauton(Team team) {
	int teamMultiplier = 1;
	if (team == BLUE) {
		teamMultiplier = 1;
	}
	else if (team == RED) {
		teamMultiplier = -1;
	}

	pros::delay(30);
	Commands::Press(C_BALL_LIFT_UP);
	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 1250);
	pros::delay(400);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -1100);
	pros::delay(400);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, teamMultiplier * (335), 800);//turn to shot
	pros::delay(400);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 100, 800);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_SHOOT, 0, 275); //shot 1
	pros::delay(500);
	//                                  
	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 670);//2nd shot
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_SHOOT, 0, 1500); //shot 2
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, teamMultiplier * (68));//1st bottom flag turn
	pros::delay(100);

	Commands::Press(C_DRIVE_LINEAR_TO, 1000);//push in
	pros::delay(1000);
	Commands::Release(C_DRIVE_LINEAR_TO);
	pros::delay(100);

	Commands::Release(C_BALL_LIFT_UP);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -100);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, teamMultiplier * 30, 500);
	pros::delay(100); 

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -1950);//back out to cap
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, -370 * teamMultiplier, 800); // rotate to platform
	pros::delay(100);

	Commands::Press(C_DRIVE_LINEAR, -127);//push in
	pros::delay(600);
	Commands::Release(C_DRIVE_LINEAR);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 1800); // drive onto platform
	pros::delay(100);
	//below is flipping the cap. doesn't work for now because of the hood
	/*
	Commands::Press(C_BALL_LIFT_DOWN);

	Commands::Press(C_DRIVE_ROTATE_TO, teamMultiplier * -300);
	pros::delay(1200);
	Commands::Release(C_DRIVE_ROTATE_TO);

	Commands::Press(C_DRIVE_LINEAR, 80);
	pros::delay(1200);
	Commands::Release(C_DRIVE_LINEAR, 80);
	Commands::Release(C_BALL_LIFT_DOWN);

	Commands::Press(C_DRIVE_LINEAR_TO, 300);
	pros::delay(300);
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(200);
	*/
	

}

void backauton(Team team) {
	int teamMultiplier = 1;
	if (team == BLUE) {
		teamMultiplier = 1;
	}
	else if (team == RED) {
		teamMultiplier = -1;
	}



	pros::delay(200);
	Commands::Press(C_DRIVE_LINEAR_TO, 1400);
	Commands::Press(C_BALL_LIFT_UP);
	pros::delay(2000);	
	Commands::Release(C_DRIVE_LINEAR_TO, 0);
	pros::delay(500);

	Commands::Press(C_FLYWHEEL_SET, 110);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -150);//drive back
	pros::delay(200);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, teamMultiplier * 320);//turn to shot
	pros::delay(200);

	Commands::Release(C_FLYWHEEL_SET);
	Commands::ExecuteUntilFinished(C_SHOOT, 0, 1000);
	pros::delay(100);

	Commands::Press(C_FLYWHEEL_SET, 127);
	pros::delay(1000);

	Commands::ExecuteUntilFinished(C_SHOOT, 0, 1000);//shoot 2
	pros::delay(200);

	Commands::Release(C_FLYWHEEL_SET);
	Commands::Release(C_BALL_LIFT_UP);
	pros::delay(100);


	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, teamMultiplier * -350);//wall
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -1200);//back into wall
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, teamMultiplier * 350);//
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 500);//move to row
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, teamMultiplier * -350, 800);//turn to platform
	pros::delay(100);


	Commands::Press(C_DRIVE_LINEAR, -100);//push in
	pros::delay(600);
	Commands::Release(C_DRIVE_LINEAR);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 1800); // drive onto platform
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 0);//back out


}


void skillsauton() {
	pros::delay(30);
	Commands::Press(C_BALL_LIFT_UP);
	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 1250);
	pros::delay(400);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -1100);
	pros::delay(400);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, -335, 800);//turn to shot
	pros::delay(400);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 100, 800);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_SHOOT, 0, 275); //shot 1
	pros::delay(500);
	//                                  
	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 670);//2nd shot
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_SHOOT, 0, 1500); //shot 2
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, -68);//1st bottom flag turn
	pros::delay(100);

	Commands::Press(C_DRIVE_LINEAR_TO, 1000);//push in
	pros::delay(1000);
	Commands::Release(C_DRIVE_LINEAR_TO);
	pros::delay(100);

	Commands::Release(C_BALL_LIFT_UP);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -100);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -500);//back out to cap 1
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, -330, 500);//turn to pick up cap 1
	pros::delay(100);

	Commands::Press(C_DRIVE_LINEAR_TO, -1000);//pick up cap 1
	pros::delay(800);
	Commands::Press(C_ARM_UP);
	pros::delay(500);
	Commands::Release(C_DRIVE_LINEAR_TO);// cap 1 acquired

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -1000);//go forward to align
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, 335, 800);//turn to store cap 1
	pros::delay(400);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, -1500);//back out to score cap 1
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, -335, 800);//turn to score cap 1 finally
	pros::delay(400);

	Commands::Press(C_DRIVE_LINEAR, 100);
	pros::delay(1000);
	Commands::Release(C_DRIVE_LINEAR);
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_LINEAR_TO, 150);//back out to cap 2
	pros::delay(100);

	Commands::ExecuteUntilFinished(C_DRIVE_ROTATE_TO, -335, 800);//turn to shot
	pros::delay(400);

	Commands::Press(C_DRIVE_LINEAR_TO, -1400);
	pros::delay(1000);
	Commands::Press(C_ARM_UP);
	pros::delay(500);
	Commands::Release(C_DRIVE_LINEAR_TO);

}

void autonomous() {
	Robot::SetBrakeMode(pros::E_MOTOR_BRAKE_HOLD);
	pros::Task updateTask(AutonomousUpdate, (void*) "i'd dab to that",
		TASK_PRIORITY_DEFAULT + 1, TASK_STACK_DEPTH_DEFAULT, "Auton Update");
	frontauton(RED);

	Commands::Clear();
	running = false;
}