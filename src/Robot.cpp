#include "Robot.h"

pros::Vision camera(13, pros::E_VISION_ZERO_CENTER);

int updateMillis = 15;

pros::Controller master = pros::Controller(pros::E_CONTROLLER_MASTER);
pros::Controller partner = pros::Controller(pros::E_CONTROLLER_PARTNER);

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

pros::Controller Robot::GetMasterController() {
    return master;
}

pros::Controller Robot::GetPartnerController() {
    return partner;
}

void Robot::SetDriveBrakeMode(pros::motor_brake_mode_e_t mode) {
	Robot::GetMotor(MotorID::DRIVE_LEFT_BACK)->GetProsMotor()->set_brake_mode(mode);
	Robot::GetMotor(MotorID::DRIVE_LEFT_FRONT)->GetProsMotor()->set_brake_mode(mode);
	Robot::GetMotor(MotorID::DRIVE_RIGHT_BACK)->GetProsMotor()->set_brake_mode(mode);
	Robot::GetMotor(MotorID::DRIVE_RIGHT_FRONT)->GetProsMotor()->set_brake_mode(mode);
}

int Robot::GetState() {
    return 0;
}

pros::Vision Robot::GetCamera() {
	return camera;
}

bool Robot::BallInSecondZone() {
    return GetSensor(SensorID::INDEXER_SECOND)->GetValue() < 2350;
}

bool Robot::BallInFirstZone() {
    return GetSensor(SensorID::INDEXER_FIRST)->GetValue() < 2300;
}

bool Robot::BallLoaded() {
    return BallInSecondZone() || BallInFirstZone();
}

void Robot::Init() {
    new Motor(MotorID::DRIVE_LEFT_BACK);
    new Motor(MotorID::DRIVE_LEFT_FRONT);
    new Motor(MotorConfig(MotorID::DRIVE_RIGHT_BACK, true, pros::E_MOTOR_GEARSET_18));
    new Motor(MotorConfig(MotorID::DRIVE_RIGHT_FRONT, true, pros::E_MOTOR_GEARSET_18));
    new Motor(MotorConfig(MotorID::FLYWHEEL, true, pros::E_MOTOR_GEARSET_06));
    new Motor(MotorID::BALL_LIFT);
    new Motor(MotorConfig(MotorID::INDEXER, false, pros::E_MOTOR_GEARSET_06));
	new Motor(MotorID::ARM);
    new AnalogSensor(SensorID::INDEXER_FIRST);
	new AnalogSensor(SensorID::INDEXER_SECOND);
	new Accelerometer(SensorID::ACCELEROMETER);
	new Gyro(SensorID::GYRO);


}

Sensor *Robot::GetSensor(SensorID id) {

    for (auto sensor : Sensor::allSensors) {
        if (sensor->id == id) {
            return sensor;
        }
    }
    throw "Sensor ID not found, it may not have been initialized";
}

Motor *Robot::GetMotor(MotorID id) {

    for (auto motor : Motor::allMotors) {
        if (motor->GetConfig().id == id) {
            return motor;
        }
    }
    throw "Motor ID not found, it may not have been initialized";
}
