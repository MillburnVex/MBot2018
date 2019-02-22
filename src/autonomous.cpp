
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

bool doubleScrape = false;

bool park = true;

bool parkAndCapScore = true;

void test() {
    for (int i = 0; i < 10; i++) {

        Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, -900);

		printf("DONE DONE DONE DONE DONE DONE DONE DONE DONE \n");

        Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0);

		printf("DONE DONE DONE DONE DONE DONE DONE DONE DONE \n");
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
    Commands::Execute(C_SHOOT, 0, 400); // shoot top flag

    Commands::Execute(C_LOAD_BALL, 0, 800);

    if (flipIfNoSecondShot && !Robot::BallLoaded()) {
        Commands::Press(C_FLYWHEEL_SET, 600);
        FlipCap();
        return false;
    }

    Commands::Press(C_FLYWHEEL_SET, 445); // prepare flywheel for stationary middle shot

    pros::delay(1000);

    Commands::Execute(C_SHOOT, 0, 400);
    Commands::Release(C_BALL_LIFT_UP);
    return true;
}

void DoubleShot(int delay = 400) {
    Commands::Execute(C_SHOOT, 0, delay); //shot 1

    Commands::Execute(C_DRIVE_LINEAR_TO, 580);// 2nd shot alignment

    Commands::Execute(C_SHOOT, 0, delay); //shot 2
}

void DoubleScrape(int timesToShake = 1) {

    Commands::Press(C_DRIVE_LINEAR, 50);
    pros::delay(500);

    Commands::Release(C_DRIVE_LINEAR);
    pros::delay(20);

    Commands::Press(C_BALL_LIFT_UP);
    Commands::Press(C_DRIVE_LINEAR, -75);
    pros::delay(500);

    Commands::Release(C_DRIVE_LINEAR);

    pros::delay(1000);
	for (int i = 0; i < timesToShake; i++) {
		Commands::Release(C_BALL_LIFT_UP);
		Commands::Press(C_BALL_LIFT_DOWN);

		pros::delay(100); // wiggle ball to break up jams

		Commands::Release(C_BALL_LIFT_DOWN);
		Commands::Press(C_BALL_LIFT_UP);
		pros::delay(450);
	}
    Commands::Execute(C_LOAD_BALL, 0, 1000);

    Commands::Release(C_BALL_LIFT_UP);
}

void FrontAuton(Team team) {
	Robot::SetCancelCommandIfNotMoving(false);

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

    Commands::Execute(C_DRIVE_LINEAR_TO, (team == BLUE ? -70 : 60));

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

        Commands::Execute(C_DRIVE_LINEAR_TO, 635); // push in, score bottom flag

        Commands::Release(C_BALL_LIFT_UP);
		pros::delay(10);
        Commands::Press(C_BALL_LIFT_DOWN);

        Commands::Execute(C_DRIVE_LINEAR_TO, -695); // back out

        if (noParkFlip) {

            Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, -30 * teamMultiplier); // rotate to cap

            Commands::Execute(C_DRIVE_LINEAR_TO, 60);

            FlipCap();

            Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 500); // rotate to bottom flag

            Commands::Press(C_DRIVE_LINEAR, 127); // toggle bottom flag

            pros::delay(1000);

            Commands::Release(C_DRIVE_LINEAR);
        } else if (park) {

            Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * (935)); // rotate before fully backing out

            Commands::Execute(C_DRIVE_LINEAR_TO, (-1420 + (team == RED ? 50 : 0))); //back out to platform

            Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 0); // rotate to platform

            Commands::Release(C_BALL_LIFT_DOWN);
            Commands::Press(C_BALL_LIFT_UP);

            Commands::Execute(C_DRIVE_LINEAR_TO, 1400); // drive onto platform

            Commands::Release(C_BALL_LIFT_UP);
        }
    }
}

void BackAuton(Team team) {
	Robot::SetCancelCommandIfNotMoving(false);

    int teamMultiplier = 1;
    if (team == BLUE) {
        teamMultiplier = 1;
    } else if (team == RED) {
        teamMultiplier = -1;
    }

    pros::delay(20);

    Commands::Press(C_BALL_LIFT_UP);
    Commands::Press(C_FLYWHEEL_SET, 555);
    Commands::Execute(C_DRIVE_LINEAR_TO, 1270); // get ball under cap

    Commands::Execute(C_DRIVE_LINEAR_TO, -170); //drive back

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 786); // turn to middle flags
    pros::delay(300);

    Commands::Execute(C_SHOOT, 0, 300); // shoot top flag

    Commands::Execute(C_SHOOT, 0, 300); // shoot middle flag

    pros::delay(200);

    Commands::Press(C_FLYWHEEL_SET, 600);

    Commands::Release(C_BALL_LIFT_UP);

    if (park) {

        Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 890); // rotate to platform

        Commands::Execute(C_DRIVE_LINEAR_TO, 1100); // park
    } else if (parkAndCapScore) {
        Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 1250); // rotate to back into cap

        Commands::Execute(C_DRIVE_LINEAR_TO, -560, 650); // pick up cap

        pros::delay(100);

        Commands::Press(C_ARM_UP);
        pros::delay(10);
        Commands::Release(C_ARM_UP);

        pros::delay(400);

        Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // get ready to back up to pole

        Commands::Execute(C_DRIVE_LINEAR_TO, -665); // back up to pole

		Commands::Press(C_BALL_LIFT_UP);
        Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -900);

        Commands::Press(C_DRIVE_LINEAR, 80); // drive into pole

        pros::delay(600);

        Commands::Release(C_DRIVE_LINEAR);

        Commands::Press(C_ARM_UP);
        pros::delay(Robot::GetUpdateMillis());
        Commands::Release(C_ARM_UP); // score cap

        pros::delay(600); // wait for score

        Commands::Execute(C_DRIVE_LINEAR_TO, -200);

        Commands::Press(C_ARM_DOWN);
        pros::delay(Robot::GetUpdateMillis());
        Commands::Release(C_ARM_DOWN); // lower arm

        Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 567); // rotate to platforms

        Commands::Execute(C_DRIVE_LINEAR_TO, 1650); // park
		Commands::Release(C_BALL_LIFT_DOWN);
    }
}


void Skills() {
	Robot::SetCancelCommandIfNotMoving(true);

    int teamMultiplier = -1;

    auto team = RED;

	pros::delay(20);
	Commands::Press(C_BALL_LIFT_UP);
	Commands::Execute(C_DRIVE_LINEAR_TO, 1250);

	Commands::Execute(C_DRIVE_LINEAR_TO, -1100);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // turn to shot

	Commands::Execute(C_DRIVE_LINEAR_TO, 50);

	DoubleShot(600);

	pros::delay(100);

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 1020); // rotate to bottom flag

    Commands::Execute(C_DRIVE_LINEAR_TO, 650); // BOTTOM FLAG ONE

    // NEXT PART: CAP, BOTTOM FLAG

    Commands::Execute(C_DRIVE_LINEAR_TO, -710); // back partially

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 920); // rotate before fully backing out

    Commands::Execute(C_DRIVE_LINEAR_TO, -600); // back out to cap

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate towards other side of field

    Commands::Press(C_DRIVE_LINEAR, -120); // back in to align with wall

    pros::delay(500);

    Commands::Release(C_DRIVE_LINEAR);

    pros::delay(100);

    Robot::ResetRotation(); // ALIGNED WITH WALL - GYROS RESET

    Commands::Execute(C_DRIVE_LINEAR_TO, 800); // drive forwards from wall

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // rotate to align with cap

    Commands::Execute(C_DRIVE_LINEAR_TO, 110); // go up close to cap

	FlipCap(); // CAP SCORED

    Commands::Execute(C_DRIVE_LINEAR_TO, -150); // back up from cap

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // prepare to align with bottom flag

    Commands::Execute(C_DRIVE_LINEAR_TO, 670); // align with bottom flag

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // turn towards bottom flag

	Commands::Press(C_BALL_LIFT_UP);
	Commands::Press(C_DRIVE_LINEAR, 120); // BOTTOM FLAG TWO

	pros::delay(1100);

	Commands::Release(C_DRIVE_LINEAR);

	pros::delay(200);

	Robot::ResetRotation(teamMultiplier * 900); // ALIGNED WITH WALL - GYROS RESET


		Commands::Execute(C_DRIVE_LINEAR_TO, -620); //  back up from wall, at the right amount to hit middle flag

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 800); // align with flag

		Commands::Execute(C_SHOOT, 0, 500); // shoot middle flag

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // re rotate correctly

		Commands::Execute(C_DRIVE_LINEAR_TO, -70);


    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -75); // face the next cap

    Commands::Execute(C_DRIVE_LINEAR_TO, 890); // drive up to cap

	FlipCap(); // FLIP CAP 2

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 910); // rotate towards bottom flag

    Commands::Press(C_DRIVE_LINEAR, 120); // BOTTOM FLAG THREE

    pros::delay(1200);

    Commands::Release(C_DRIVE_LINEAR);

    pros::delay(300);

    Robot::ResetRotation(teamMultiplier * 900); // ALIGNED WITH WALL - GYROS RESET

		Commands::Execute(C_DRIVE_LINEAR_TO, -640); //  back up from wall, at the right amount to hit middle flag

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 800); // align with flag

		Commands::Execute(C_SHOOT, 0, 500); // shoot middle flag

		Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 900); // re rotate correctly

		Commands::Execute(C_DRIVE_LINEAR_TO, -680);

    // NEXT PART: 2 HIGH CAPS, PARK

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate to face other wall

	Commands::Press(C_DRIVE_LINEAR, 127); // ALIGN WITH WALL

	pros::delay(800);

	Commands::Release(C_DRIVE_LINEAR);

	pros::delay(200);

	Robot::ResetRotation(); // ALIGNED WITH WALL - GYROS RESET

	Commands::Execute(C_DRIVE_LINEAR_TO, -150); // back up from wall

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -950); // rotate towards back side

	Commands::Execute(C_DRIVE_LINEAR_TO, 2040); // drive over to other side

	Commands::Press(C_DRIVE_LINEAR, 127); // ALIGN WITH WALL

	pros::delay(400);

	Commands::Release(C_DRIVE_LINEAR);

	pros::delay(100);

	Robot::ResetRotation(teamMultiplier * -900); // ALIGNED WITH WALL - GYROS RESET

	Commands::Execute(C_DRIVE_LINEAR_TO, -120); // back up from wall

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate so that back is facing cap

	Commands::Press(C_DRIVE_LINEAR, 127); // ALIGN WITH WALL

	pros::delay(600);

	Commands::Release(C_DRIVE_LINEAR);

	pros::delay(100);

	Robot::ResetRotation();

	Commands::Execute(C_DRIVE_LINEAR_TO, -200); // back up from wall

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -10); // rotate so that back is facing cap

	Commands::Execute(C_DRIVE_LINEAR_TO, -900); // back up into cap

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 10); // aim more precisely

	Commands::Press(C_DRIVE_LINEAR, -60); // slowly back up into cap

	pros::delay(400);

	Commands::Release(C_DRIVE_LINEAR);

	Commands::Press(C_ARM_UP); // lift cap up
	pros::delay(10);
	Commands::Release(C_ARM_UP);

	pros::delay(600); // wait for cap to be up

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * 120); // rotate towards opposite side

	Commands::Execute(C_DRIVE_LINEAR_TO, 453); // drive forward to be ready for pole

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, teamMultiplier * -900); // rotate to pole

	Commands::Press(C_DRIVE_LINEAR, 80); // drive into pole

	pros::delay(600);

	Commands::Release(C_DRIVE_LINEAR);

	Robot::ResetRotation(-900 * teamMultiplier); // ROBOT ALIGNED, GYROS RESET

	Commands::Execute(C_FLYWHEEL_SET, 560); // prepare to hit far flags
	Commands::Press(C_ARM_UP);
	pros::delay(10);
	Commands::Release(C_ARM_UP); // score cap

	pros::delay(600); // wait for score
	
	Commands::Execute(C_DRIVE_LINEAR_TO, -200); // back up

	Commands::Press(C_ARM_DOWN); // put arm down
	pros::delay(10);
	Commands::Release(C_ARM_DOWN);

	Commands::Press(C_BALL_LIFT_UP);
	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 1400 * teamMultiplier);

	Commands::Execute(C_DRIVE_LINEAR_TO, 570); // hopefully get ball from under cap

	Commands::Execute(C_DRIVE_LINEAR_TO, -100); // back up from cap

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 790 * teamMultiplier); // prepare to score flags

	Commands::Execute(C_LOAD_BALL, 0, 1000); // guarantee ball is loaded

	Commands::Execute(C_SHOOT, 0, 400);

	Commands::Execute(C_SHOOT, 0, 400);

    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 0); // rotate towards far side

	Commands::Execute(C_DRIVE_LINEAR_TO, 800); // go towards far side

    Commands::Press(C_DRIVE_LINEAR, 127); // ALIGN WITH WALL

    pros::delay(500);

    Commands::Release(C_DRIVE_LINEAR);

    pros::delay(100);

    Robot::ResetRotation(); // ALIGNED WITH WALL - GYROS RESET

    Commands::Execute(C_DRIVE_LINEAR_TO, -100);

	Commands::Press(C_BALL_LIFT_DOWN);
    Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 900 * teamMultiplier);

    Commands::Execute(C_DRIVE_LINEAR_TO, 670);
	Commands::Release(C_BALL_LIFT_DOWN);
	pros::delay(10);
	Commands::Press(C_BALL_LIFT_UP);

	Commands::Execute(C_DRIVE_ROTATE_TO_ABSOLUTE, 1800 * teamMultiplier); // rotate towards platforms

	Commands::Execute(C_DRIVE_LINEAR_TO, 2600); // park

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
	FrontAuton(RED);
    pros::delay(100);
    Commands::Clear();
    running = false;

}