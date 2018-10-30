#include "BotMotor.h"
#include "Robot.h"

std::vector<BotMotor *> BotMotor::motors;

void RealMotor::SetPositionAbsolute(double pos, int vel) {
    motor->move_absolute(pos, vel);
}

void RealMotor::SetPositionRelative(double pos, int vel) {
    motor->move_relative(pos, vel);
}

void RealMotor::SetVelocity(int8_t vel) {
    motor->move_velocity(vel);
}

void RealMotor::SetVoltage(int8_t voltage) {
    motor->move_voltage(voltage);
}

void RealMotor::SetPID(pros::motor_pid_s_t pid) {
    motor->set_pos_pid(pid);
    motor->set_vel_pid(pid);
}

pros::motor_pid_full_s_t RealMotor::GetPID() {
    return motor->get_vel_pid();
}

BotMotor::BotMotor(BotMotorConfig config) : config(config) {
    motors.push_back(this);
};
