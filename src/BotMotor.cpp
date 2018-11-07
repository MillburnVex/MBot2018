#include "BotMotor.h"
#include "Robot.h"

std::vector<BotMotor *> BotMotor::allMotors;

void RealMotor::vectorPositionAbsolute(double pos, int vel) {
    motor->move_absolute(pos, vel);
}

void RealMotor::vectorPositionRelative(double pos, int vel) {
    motor->move_relative(pos, vel);
}

void RealMotor::SetVelocity(int16_t vel) {
    motor->move_velocity(vel);
}

void RealMotor::vectorVoltage(int8_t voltage) {
    motor->move(voltage);
}

void RealMotor::vectorPID(pros::motor_pid_s_t pid) {
    motor->set_pos_pid(pid);
    motor->set_vel_pid(pid);
}

pros::motor_pid_full_s_t RealMotor::GetPID() {
    return motor->get_vel_pid();
}

BotMotor::BotMotor(BotMotorConfig config) : config(config) {
    allMotors.push_back(this);
};
