#include "Robot.h"

int Robot::GetState() {
    return 0;
}

void Robot::Init() {
    RealMotor(BotMotorID::CLAW);
    RealMotor(BotMotorID::DRIVE_LEFT_BACK);
    RealMotor(BotMotorID::DRIVE_LEFT_FRONT);
    RealMotor(BotMotorID::DRIVE_RIGHT_BACK);
    RealMotor(BotMotorID::DRIVE_RIGHT_FRONT);
    RealMotor(BotMotorID::FLYWHEEL);
    RealMotor(BotMotorID::CAP_LIFT);
    RealMotor(BotMotorID::BALL_LIFT);
}

BotMotor * Robot::GetMotor(BotMotorID id) {
    for (auto motor : BotMotor::motors) {
        if (motor->GetConfig().id == id) {
            return motor;
        }
    }
    throw "Motor ID not found, it may not have been initialized";
}
