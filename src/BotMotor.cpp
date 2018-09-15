#include "BotMotor.h"

BotMotorConfig::BotMotorConfig(std::uint8_t id, bool reverse, pros::motor_gearset_e_t gearset) {
    this->id = id;
    this->reverse = reverse;
    this->gearset = gearset;
}

void RealMotor::setPositionAbsolute(double pos, int vel) {
    motor->move_absolute(pos, vel);
}

void RealMotor::setPositionRelative(double pos, int vel) {
    motor->move_relative(pos, vel);
}

void RealMotor::setVelocity(int8_t vel) {
    motor->move_velocity(vel);
}

void RealMotor::setVoltage(int8_t voltage) {
    motor->move_voltage(voltage);
}

void RealMotor::setPID(pros::motor_pid_s_t pid) {
    motor->set_pos_pid(pid);
    motor->set_vel_pid(pid);
}

pros::motor_pid_full_s_t RealMotor::getPID() {
    return motor->get_vel_pid();
}