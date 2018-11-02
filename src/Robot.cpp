#include "Robot.h"
#include "BotComponent.h"


int Robot::GetState() {
    return 0;
}

void Robot::Init() {
    new RealMotor(BotMotorID::CLAW);
	new RealMotor(BotMotorID::DRIVE_LEFT_BACK);
	new RealMotor(BotMotorID::DRIVE_LEFT_FRONT);
	new RealMotor(BotMotorID::DRIVE_RIGHT_BACK);
	new RealMotor(BotMotorID::DRIVE_RIGHT_FRONT);
	new RealMotor(BotMotorConfig(BotMotorID::FLYWHEEL, false, pros::E_MOTOR_GEARSET_06));
	new RealMotor(BotMotorID::CAP_LIFT);
	new RealMotor(BotMotorID::BALL_LIFT);
}

BotMotor *Robot::GetMotor(BotMotorID id) {

	for (auto motor : BotMotor::motors) {
		if (motor->GetConfig().id == id) {
			return motor;
		}
	}
	throw "Motor ID not found, it may not have been initialized";
}
