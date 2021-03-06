#include "Robot.h"

pros::Vision camera(13, pros::E_VISION_ZERO_CENTER);

int updateMillis = 5;

int rotationOffset = 0;

bool cancelCommandIfNotMoving = false;

pros::Controller master = pros::Controller(pros::E_CONTROLLER_MASTER);
pros::Controller partner = pros::Controller(pros::E_CONTROLLER_PARTNER);

Team team = RED;
Strategy strat = FRONT_PARK;

bool manualMode = false;

int Robot::GetUpdateMillis() {
    return updateMillis;
}

Team Robot::GetTeam() {
    return team;
}

Strategy Robot::GetAutonStrategy() {
    return strat;
}

bool Robot::IsInManualMode() {
    return manualMode;
}

void Robot::SetTeam(Team t) {
    team = t;
}

void Robot::SetCancelCommandIfNotMoving(bool t) {
	cancelCommandIfNotMoving = t;
}

bool Robot::ShouldCancelCommandIfNotMoving() {
	return cancelCommandIfNotMoving;
}

void Robot::SetAutonStrategy(Strategy newStrat) {
    strat = newStrat;
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

pros::Vision Robot::GetCamera() {
    return camera;
}

bool Robot::BallInSecondZone() {
    return GetSensor(SensorID::INDEXER_SECOND)->GetValue() < 2350;
}

bool Robot::BallInFirstZone() {
    return GetSensor(SensorID::INDEXER_FIRST)->GetValue() < 2300;
}

int Robot::GetRotation() {
    return (int) (((float) (GetSensor(GYRO)->GetValue() + GetSensor(GYRO_2)->GetValue())) / 2.0f) + rotationOffset;
}

void Robot::ResetRotation(int offset) {
    auto gyro1 = static_cast<Gyro *>(GetSensor(GYRO));
    auto gyro2 = static_cast<Gyro *>(GetSensor(GYRO_2));
    gyro1->Reset();
    gyro2->Reset();
    rotationOffset = offset;
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
    new Gyro(SensorID::GYRO_2);
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
