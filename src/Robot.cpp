#include "Robot.h"


pros::Vision camera(4, pros::E_VISION_ZERO_CENTER);

int Robot::GetState() {
    return 0;
}

pros::Vision Robot::GetCamera() {
	return camera;
}

void Robot::Init() {
    new RealMotor(BotMotorConfig(BotMotorID::CLAW, false, pros::E_MOTOR_GEARSET_36));
    new RealMotor(BotMotorID::DRIVE_LEFT_BACK);
    new RealMotor(BotMotorID::DRIVE_LEFT_FRONT);
    new RealMotor(BotMotorID::DRIVE_RIGHT_BACK);
    new RealMotor(BotMotorID::DRIVE_RIGHT_FRONT);
    new RealMotor(BotMotorConfig(BotMotorID::FLYWHEEL, false, pros::E_MOTOR_GEARSET_06));
    new RealMotor(BotMotorConfig(BotMotorID::CAP_LIFT, false, pros::E_MOTOR_GEARSET_36));
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
