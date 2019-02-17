
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
    for (int i = 0; i < 10; i++) {

        Commands::Execute(C_DRIVE_LINEAR_TO, 1500);

        Commands::Execute(C_DRIVE_LINEAR_TO, -1500);
    }


}

void FlipCap() {
	Commands::Press(C_BALL_LIFT_DOWN);

	Commands::Press(C_DRIVE_LINEAR, 55);

	pros::delay(1200);

	Commands::Release(C_DRIVE_LINEAR);

	pros::delay(50);

	Commands::Release(C_BALL_LIFT_DOWN);
}

bool StationaryDoubleShot(bool flipIfNoSecondShot = true) {
	Commands::Press(C_BALL_LIFT_UP);
    Commands::Execute(C_SHOOT, 0, 300); // shoot top flag

	Commands::Execute(C_LOAD_BALL, 0, 800);

	if (flipIfNoSecondShot && !Robot::BallLoaded()) {
		Commands::Press(C_FLYWHEEL_SET, 600);
		FlipCap();
		return false;
	}

    Commands::Press(C_FLYWHEEL_SET, 445); // prepare flywheel for stationary middle shot

    pros::delay(1000);

    Commands::Execute(C_SHOOT, 0, 300);
	Commands::Release(C_BALL_LIFT_UP);
	return true;
}

void DoubleShot(int delay = 0) {
    Commands::Execute(C_SHOOT, 0, 300); //shot 1

	pros::delay(delay);

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

	Commands::Release(C_BALL_LIFT_UP);
	Commands::Press(C_BALL_LIFT_DOWN);

	pros::delay(100); // wiggle ball to break up jams

	Commands::Release(C_BALL_LIFT_DOWN);

	Commands::Execute(C_LOAD_BALL, 0, 1000);

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

    Commands::Execute(C_DRIVE_LINEAR_TO, 1250);

    Commands::Execute(C_DRIVE_LINEAR_TO, -1100);

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // turn to shot

	Commands::Execute(C_DRIVE_LINEAR_TO, (team == BLUE ? -50 : 60));

    DoubleShot();

    pros::delay(100);
   
	if (doubleScrape) {
		Commands::Execute(C_FLYWHEEL_SET, 550);

		Commands::Release(C_BALL_LIFT_UP);

		pros::delay(1000); // wait for balls to go past

		Commands::Execute(C_DRIVE_LINEAR_TO, -750); // go back to behind cap

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 487); // rotate to align with cap

		Commands::Execute(C_DRIVE_LINEAR_TO, 460); // go up close to cap

		DoubleScrape();

		// if no ball is loaded after some time, we should reverse intake and flip cap

		if (Robot::BallLoaded()) {

			Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 420); // rotate to align flat

			Commands::Execute(C_DRIVE_LINEAR_TO, 100); // go up a bit to flag

			if (StationaryDoubleShot()) {
				// successful double shot, try to flip the cap
				Commands::Press(C_BALL_LIFT_DOWN);
				Commands::Press(C_DRIVE_LINEAR, 100);
				pros::delay(500);
				Commands::Release(C_DRIVE_LINEAR);
			}
		} else {
			// no ball is loaded. try to flip the cap

			Commands::Release(C_BALL_LIFT_UP);
			
			FlipCap();
		}
	} else {

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 1060); // 1st bottom flag turn

		Commands::Execute(C_DRIVE_LINEAR_TO, 620); // push in, score bottom flag

		Commands::Release(C_BALL_LIFT_UP);
		Commands::Press(C_BALL_LIFT_DOWN);

		Commands::Execute(C_DRIVE_LINEAR_TO, -680); // back out

		if (noParkFlip) {

			Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate to cap

			FlipCap();

			Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 450); // rotate to bottom flag

			Commands::Press(C_DRIVE_LINEAR, 127); // toggle bottom flag

			pros::delay(1000);

			Commands::Release(C_DRIVE_LINEAR);
		}
		else if (park) {

			Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (935)); // rotate before fully backing out

			Commands::Execute(C_DRIVE_LINEAR_TO, (-1450)); //back out to platform

			Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -30); // rotate to platform

			Commands::Release(C_BALL_LIFT_DOWN);
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

	Commands::Press(C_BALL_LIFT_UP);

	Commands::Execute(C_DRIVE_LINEAR_TO, 1250); // grab ball

	Commands::Execute(C_DRIVE_LINEAR_TO, -1130);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // turn to shot

	Commands::Execute(C_DRIVE_LINEAR_TO, 60); // align with flags

	DoubleShot(500); // SHOTS ONE AND TWO

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 1020); // rotate to bottom flag

	Commands::Execute(C_DRIVE_LINEAR_TO, 620); // BOTTOM FLAG ONE

	// NEXT PART: DOUBLE SCRAPE, 2 FLAGS, CAP, BOTTOM FLAG

	Commands::Execute(C_DRIVE_LINEAR_TO, -680); // back partially

	Commands::Release(C_BALL_LIFT_UP);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (920)); // rotate before fully backing out

	Commands::Execute(C_DRIVE_LINEAR_TO, -600); // back out to cap

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate towards other side of field

	Commands::Press(C_DRIVE_LINEAR, -100); // back in to align with wall

	pros::delay(600);

	Commands::Release(C_DRIVE_LINEAR);

	pros::delay(300);

	Robot::ResetRotation(); // ALIGNED WITH WALL - GYROS RESET

	//Components::ResetInitialPositions(); // RESET DRIVE MOTOR POSITIONS

	Commands::Execute(C_DRIVE_LINEAR_TO, 265); // drive forwards from wall

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 510); // rotate to align with cap

	Commands::Execute(C_DRIVE_LINEAR_TO, 370); // go up close to cap

	DoubleScrape();

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 490); // rotate to align with middle flags

	Commands::Execute(C_DRIVE_LINEAR_TO, 100); // go up a bit to flag

	StationaryDoubleShot(false); // SHOTS THREE AND FOUR

	Commands::Execute(C_DRIVE_LINEAR_TO, -100); // back up for cap flip

	FlipCap(); // FLIP CAP

	Commands::Execute(C_DRIVE_LINEAR_TO, -200); // back up from cap

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // face the other side of the field

	Commands::Execute(C_DRIVE_LINEAR_TO, 600); // prepare to hit bottom flag

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 920); // rotate towards bottom flag

	Commands::Press(C_DRIVE_LINEAR, 100); // BOTTOM FLAG TWO

	pros::delay(1500);

	Commands::Release(C_DRIVE_LINEAR);

	pros::delay(300);

	Robot::ResetRotation(900 * teamMultiplier); // ALIGNED WITH WALL - GYROS RESET

	//Components::ResetInitialPositions(); // RESET DRIVE MOTOR POSITIONS

	// NEXT PART: FLIP CAP, PARK

	Commands::Execute(C_DRIVE_LINEAR_TO, -900); // back up to cap

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate to cap

	Commands::Execute(C_DRIVE_LINEAR_TO, 800); // go up to cap

	FlipCap(); // FLIP CAP

	Commands::Execute(C_DRIVE_LINEAR_TO, -300); // back up from cap

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -900); // rotate towards platforms

	Commands::Execute(C_DRIVE_LINEAR_TO, 500); // away from flags

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // face towards wall

	Commands::Press(C_DRIVE_LINEAR, 100); // align with wall

	pros::delay(2000);

	Commands::Release(C_DRIVE_LINEAR);

	Robot::ResetRotation(); // ALIGNED WITH WALL - GYROS RESET

	//Components::ResetInitialPositions(); // RESET DRIVE MOTOR POSITIONS

	Commands::Execute(C_DRIVE_LINEAR_TO, -250);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -900); // rotate towards platforms

	Commands::Execute(C_DRIVE_LINEAR_TO, 600);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -1800); // prepare to go up on platforms

	Commands::Execute(C_DRIVE_LINEAR_TO, 2400); // park

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
    pros::Task updateTask(AutonomousUpdate, (void *) "i'd dab to that",
                          TASK_PRIORITY_DEFAULT + 1, TASK_STACK_DEPTH_DEFAULT, "Auton Update");
    /*
    if(Robot::GetAutonPosition() == FRONT) {
        FrontAuton(Robot::GetTeam());
    } else if(Robot::GetAutonPosition() == BACK) {
        BackAuton(Robot::GetTeam());
    }
    */
	pros::delay(20);
	FrontAuton(BLUE);
	pros::delay(100);
    Commands::Clear();
    running = false;

}