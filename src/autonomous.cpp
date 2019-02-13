
#include "../include/main.h"
#include "../include/api.h"
#include "Component.h"
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

bool noParkFlip = false;

bool doubleScrape = true;

bool park = false;

void test() {
	for (int i = 0; i < 5; i++) {

		Commands::Execute(C_DRIVE_LINEAR_TO, 1000); // turn to shot

		Commands::Execute(C_DRIVE_LINEAR_TO, -1000); // turn to shot
	}

	

}

void DoubleShot() {
	Commands::Execute(C_SHOOT, 0, 300); //shot 1

	Commands::Execute(C_DRIVE_LINEAR_TO, 580);//2nd shot

	Commands::Execute(C_SHOOT, 0, 300); //shot 2
}

void DoubleScrape() {
	Commands::Press(C_DRIVE_LINEAR, 50);
	pros::delay(400);

	Commands::Release(C_DRIVE_LINEAR);
	pros::delay(20);

	Commands::Press(C_BALL_LIFT_UP);
	Commands::Press(C_DRIVE_LINEAR, -75);
	pros::delay(300);

	Commands::Release(C_DRIVE_LINEAR);
	pros::delay(800);
}

void FrontAuton(Team team) {
	int teamMultiplier = 1;
	if (team == BLUE) {
		teamMultiplier = 1;
	}
	else if (team == RED) {
		teamMultiplier = -1;
	}

	pros::delay(20);

	Commands::Press(C_BALL_LIFT_UP);

	Commands::Execute(C_DRIVE_LINEAR_TO, 1250);

	Commands::Execute(C_DRIVE_LINEAR_TO, -1100);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // turn to shot	
	
	DoubleShot();

	pros::delay(100);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 1000);//1st bottom flag turn

	Commands::Execute(C_DRIVE_LINEAR_TO, 585); // push in

	Commands::Execute(C_DRIVE_LINEAR_TO, -645); // back out a little bit

    Commands::Release(C_BALL_LIFT_UP);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (920)); // rotate before fully backing out

	if (doubleScrape) {
		Commands::Release(C_BALL_LIFT_UP);
		Commands::Release(C_BALL_LIFT_DOWN);

		if (Robot::BallLoaded()) {
			Commands::Execute(C_SHOOT, 150);
		}

		Commands::Execute(C_DRIVE_LINEAR_TO, -600); // go back to behind cap
		 
		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 450); // rotate to align with cap

		Commands::Execute(C_DRIVE_LINEAR_TO, 360); // go up close to cap

		//Commands::Execute(C_FLYWHEEL_SET, 565, 0);

		DoubleScrape();

		Commands::Execute(C_DRIVE_LINEAR_TO, teamMultiplier * -400); // rotate to align with cap

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 0); // rotate to align flat

		Commands::Execute(C_DRIVE_LINEAR_TO, 1800); // drive to angle

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // rotate to align flat



		//Commands::Execute(C_LOAD_BALL, 0);


		//Commands::Execute(C_DRIVE_LINEAR_TO, 300); // align with flag



		DoubleShot();
		
	} else if (noParkFlip) {

		Commands::Press(C_BALL_LIFT_DOWN);

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate to platform

		Commands::Press(C_DRIVE_LINEAR, 55);

		pros::delay(1200);

		Commands::Release(C_DRIVE_LINEAR);

		pros::delay(50);


		Commands::Release(C_BALL_LIFT_DOWN);
	}
	else if (park) {
		Commands::Execute(C_DRIVE_LINEAR_TO, (-1410)); //back out to platform

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate to platform

		Commands::Press(C_BALL_LIFT_UP);

		Commands::Execute(C_DRIVE_LINEAR_TO, 1400); // drive onto platform

		Commands::Release(C_BALL_LIFT_UP);

	}
}

void BackAuton(Team team) {
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

	Commands::Execute(C_DRIVE_LINEAR_TO, -150);//drive back
	pros::delay(200);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 320);//turn to shot
	pros::delay(200);

	Commands::Release(C_FLYWHEEL_SET);
	Commands::Execute(C_SHOOT, 0, 1000);
	pros::delay(100);

	Commands::Press(C_FLYWHEEL_SET, 127);
	pros::delay(1000);

	Commands::Execute(C_SHOOT, 0, 1000);//shoot 2
	pros::delay(200);

	Commands::Release(C_FLYWHEEL_SET);
	Commands::Release(C_BALL_LIFT_UP);
	pros::delay(100);


	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -350);//wall
	pros::delay(100);

	Commands::Execute(C_DRIVE_LINEAR_TO, -1200);//back into wall
	pros::delay(100);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 350);//
	pros::delay(100);

	Commands::Execute(C_DRIVE_LINEAR_TO, 500);//move to row
	pros::delay(100);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -350, 800);//turn to platform
	pros::delay(100);


	Commands::Press(C_DRIVE_LINEAR, -100);//push in
	pros::delay(600);
	Commands::Release(C_DRIVE_LINEAR);

	Commands::Execute(C_DRIVE_LINEAR_TO, 1800); // drive onto platform
	pros::delay(100);

	Commands::Execute(C_DRIVE_LINEAR_TO, 0);//back out


}


void skillsauton() {
	int teamMultiplier = -1;

	auto team = RED;

	pros::delay(20);

	Commands::Press(C_BALL_LIFT_UP);

	Commands::Execute(C_DRIVE_LINEAR_TO, 1250);

	Commands::Execute(C_DRIVE_LINEAR_TO, -1100);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (330 - ((team == RED) ? 36 : 0))); // turn to shot

	Commands::Execute(C_DRIVE_LINEAR_TO, 55 + ((team == RED ? 60 : 0))); // line up with flag

	DoubleShot();

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 78);//1st bottom flag turn

	Commands::Execute(C_DRIVE_LINEAR_TO, 495); // push in

	Commands::Execute(C_DRIVE_LINEAR_TO, -545); // back out a little bit

	Commands::Release(C_BALL_LIFT_UP);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (-62)); // rotate before fully backing out


		Commands::Press(C_BALL_LIFT_DOWN);

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, -(310) * teamMultiplier); // rotate to cap

		Commands::Press(C_DRIVE_LINEAR, 65);

		pros::delay(1000);//flip cap

		Commands::Release(C_BALL_LIFT_DOWN);
		Commands::Release(C_DRIVE_LINEAR);

		pros::delay(1000);//wait for cap to stop moving

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (-40));


		Commands::Press(C_DRIVE_LINEAR, 65);//push cap out of the way

		pros::delay(3000);

		Commands::Release(C_DRIVE_LINEAR);

		pros::delay(1000);


		Commands::Execute(C_DRIVE_LINEAR_TO, -355); //back out

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (350)); //turn to bottom flag	
																	  
		Commands::Press(C_DRIVE_LINEAR, 100);

		pros::delay(1000);

		Commands::Release(C_DRIVE_LINEAR);

		pros::delay(400);

		Commands::Execute(C_DRIVE_LINEAR_TO, -375);//pull back

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (-330)); //turn to go towards far side of field	

		Commands::Execute(C_DRIVE_LINEAR_TO, 880);// drive  to far side of the field in preparation for flipping cap 2
		
		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (-190)); //turn to cap 2 	

		Commands::Execute(C_DRIVE_LINEAR_TO, 100);// go forward up to cap 2

		Commands::Press(C_BALL_LIFT_DOWN); // flip cap 2 below

		Commands::Press(C_DRIVE_LINEAR, 65);

		pros::delay(1500);

		Commands::Release(C_DRIVE_LINEAR);

		Commands::Release(C_BALL_LIFT_DOWN); // end flip cap 2

		pros::delay(800);

		Commands::Execute(C_DRIVE_LINEAR_TO, -100);//pull back from cap 2

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (490)); //turn to bottom flag 3	

		Commands::Press(C_DRIVE_LINEAR, 100); // push in bottom flag 3

		pros::delay(1500);

		Commands::Release(C_DRIVE_LINEAR); // end push in bottom flag 3

		pros::delay(400);

		Commands::Execute(C_DRIVE_LINEAR_TO, -650); // back out from flag 3

		while (Robot::BallLoaded()) { // get rid of extra balls
			Commands::Execute(C_SHOOT, 0, 400);
		}

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, -330); // rotate to be ready to back into wall

		Commands::Press(C_DRIVE_LINEAR, -90); // drive backwards into the wall to align

		pros::delay(3000);

		Commands::Release(C_DRIVE_LINEAR); // aligned with wall

		Commands::Execute(C_DRIVE_LINEAR_TO, 150); // drive forward to rotate

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 330); // rotate to go backwards to platform

		Commands::Execute(C_DRIVE_LINEAR_TO, (-1350)); //back out to platform

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, (300) * teamMultiplier); // rotate to platform

		Commands::Press(C_BALL_LIFT_UP);

		Commands::Execute(C_DRIVE_LINEAR_TO, 2400); // drive onto platform

		Commands::Release(C_BALL_LIFT_UP);
}

void AutonomousUpdate(void *params) {
    uint32_t time = pros::millis();
    while (running) {
        Commands::Update();
        Components::Update();
        pros::Task::delay_until(&time, Robot::GetUpdateMillis());
    }
}

void autonomous() {

	Robot::SetDriveBrakeMode(pros::E_MOTOR_BRAKE_BRAKE);
	pros::Task updateTask(AutonomousUpdate, (void*) "i'd dab to that",
		TASK_PRIORITY_DEFAULT + 1, TASK_STACK_DEPTH_DEFAULT, "Auton Update");
	/*
	if(Robot::GetAutonPosition() == FRONT) {
		FrontAuton(Robot::GetTeam());
	} else if(Robot::GetAutonPosition() == BACK) {
		BackAuton(Robot::GetTeam());
	}
	*/
	//test();
	FrontAuton(BLUE);
	pros::delay(100);
    Commands::Clear();
    running = false;
	
}