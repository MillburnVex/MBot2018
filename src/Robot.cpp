#include "Robot.h"


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
    new SensorButton(SensorID::BUTTON_BALL_LIFT_UPPER);
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
