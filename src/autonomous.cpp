
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

        Commands::Execute(C_DRIVE_LINEAR_TO, 1000);

        Commands::Execute(C_DRIVE_LINEAR_TO, -1000);
    }


}

void StationaryDoubleShot() {
    Commands::Execute(C_SHOOT, 0, 300); // shoot top flag

	pros::delay(10);

    Commands::Press(C_FLYWHEEL_SET, 340); // prepare flywheel for stationary middle shot

    pros::delay(1000);

    Commands::Execute(C_SHOOT, 0, 300);
}

void DoubleShot() {
    Commands::Execute(C_SHOOT, 0, 300); //shot 1

    Commands::Execute(C_DRIVE_LINEAR_TO, 580);// 2nd shot alignment

    Commands::Execute(C_SHOOT, 0, 300); //shot 2
}

void DoubleScrape() {

    Commands::Press(C_DRIVE_LINEAR, 50);
    pros::delay(500);

    Commands::Release(C_DRIVE_LINEAR);
    pros::delay(20);

	Commands::Press(C_BALL_LIFT_UP);
    Commands::Press(C_DRIVE_LINEAR, -75);
    pros::delay(500);

    Commands::Release(C_DRIVE_LINEAR);
    pros::delay(1000);
	Commands::Execute(C_LOAD_BALL, 0);
	Commands::Release(C_BALL_LIFT_UP);
}

void FrontAuton(Team team) {
    int teamMultiplier = 1;
    if (team == BLUE) {
        teamMultiplier = 1;
    } else if (team == RED) {
        teamMultiplier = -1;  
    }

    pros::delay(20);

    Commands::Press(C_BALL_LIFT_UP);

    Commands::Execute(C_DRIVE_LINEAR_TO, 1220);

    Commands::Execute(C_DRIVE_LINEAR_TO, -1100);

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // turn to shot

    DoubleShot();

    pros::delay(100);
   
	if (doubleScrape) {
		Commands::Execute(C_FLYWHEEL_SET, 550);

		Commands::Release(C_BALL_LIFT_UP);

		Commands::Execute(C_DRIVE_LINEAR_TO, -700); // go back to behind cap

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 480); // rotate to align with cap

		Commands::Execute(C_DRIVE_LINEAR_TO, 390); // go up close to cap

		DoubleScrape();

		// if no ball is loaded after some time, we should reverse intake and flip cap

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 420); // rotate to align flat

		Commands::Execute(C_DRIVE_LINEAR_TO, 100); // rotate to align with cap

		StationaryDoubleShot();
	}else{

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 1060); // 1st bottom flag turn

		Commands::Execute(C_DRIVE_LINEAR_TO, 585); // push in, score bottom flag

		Commands::Execute(C_DRIVE_LINEAR_TO, -645); // back out

		Commands::Release(C_BALL_LIFT_UP);

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (920)); // rotate before fully backing out
		
		if (noParkFlip) {

			Commands::Press(C_BALL_LIFT_DOWN);

			Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate to cap

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
}

void BackAuton(Team team) {
    int teamMultiplier = 1;
    if (team == BLUE) {
        teamMultiplier = 1;
    } else if (team == RED) {
        teamMultiplier = -1;
    }

    pros::delay(20);

    Commands::Press(C_BALL_LIFT_UP);
    Commands::Execute(C_DRIVE_LINEAR_TO, 1250); // get ball under cap

    Commands::Execute(C_FLYWHEEL_TOGGLE_SLOW, 0, 0);

    Commands::Execute(C_DRIVE_LINEAR_TO, -150); //drive back

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 890); // turn to middle flags

    Commands::Execute(C_SHOOT, 0, 300); // shoot top flag

    Commands::Execute(C_SHOOT, 0, 300); // shoot middle flag

    Commands::Execute(C_FLYWHEEL_TOGGLE_SLOW, 0, 0);

    Commands::Release(C_BALL_LIFT_UP);

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


void Skills() {
    int teamMultiplier = -1;

    auto team = RED;

    pros::delay(20);

	FrontAuton(RED);
}

void AutonomousUpdate(void *params) {
    uint32_t time = pros::millis();
    while (running) {
        Commands::Update();
        Components::Update();
		printf("time %d\n", pros::millis());
        pros::Task::delay_until(&time, Robot::GetUpdateMillis());
    }
}

void autonomous() {

    Robot::SetDriveBrakeMode(pros::E_MOTOR_BRAKE_BRAKE);
    pros::Task updateTask(AutonomousUpdate, (void *) "i'd dab to that",
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
    Commands::Clear();
    running = false;

}