#include "Robot.h"

pros::Vision camera(13, pros::E_VISION_ZERO_CENTER);

int updateMillis = 15;

Team team = RED;
Position autonPosition = FRONT;

bool manualMode = false;

int Robot::GetUpdateMillis() {
	return updateMillis;
}

Team Robot::GetTeam() {
	return team;
}

Position Robot::GetAutonPosition() {
	return autonPosition;
}

bool Robot::IsInManualMode() {
    return manualMode;
}

void Robot::SetTeam(Team t) {
	team = t;
}

void Robot::SetAutonPosition(Position pos) {
	autonPosition = pos;
}

void Robot::SetBrakeMode(pros::motor_brake_mode_e_t mode) {
	Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->GetProsMotor()->set_brake_mode(mode);
	Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetProsMotor()->set_brake_mode(mode);
	Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->GetProsMotor()->set_brake_mode(mode);
	Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetProsMotor()->set_brake_mode(mode);
}

int Robot::GetState() {
    return 0;
}

pros::Vision Robot::GetCamera() {
	return camera;
}

void Robot::Init() {
    new RealMotor(BotMotorID::DRIVE_LEFT_BACK);
    new RealMotor(BotMotorID::DRIVE_LEFT_FRONT);
    new RealMotor(BotMotorID::DRIVE_RIGHT_BACK);
    new RealMotor(BotMotorID::DRIVE_RIGHT_FRONT);
    new RealMotor(BotMotorConfig(BotMotorID::FLYWHEEL, false, pros::E_MOTOR_GEARSET_06));
    new RealMotor(BotMotorID::BALL_LIFT);
    (new RealMotor(BotMotorConfig(BotMotorID::INDEXER, false, pros::E_MOTOR_GEARSET_06)))->GetProsMotor()->set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	new RealMotor(BotMotorID::ARM);
    new SensorButton(SensorID::BUTTON_INDEXER);
}

Sensor *Robot::GetSensor(SensorID id) {

    for (auto sensor : Sensor::allSensors) {
        if (sensor->id == id) {
            return sensor;
        }
    }
    throw "Sensor ID not found, it may not have been initialized";
}

BotMotor *Robot::GetMotor(BotMotorID id) {

    for (auto motor : BotMotor::allMotors) {
        if (motor->GetConfig().id == id) {
            return motor;
        }
    }
    throw "Motor ID not found, it may not have been initialized";
}
