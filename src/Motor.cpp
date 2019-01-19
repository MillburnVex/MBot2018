#include "Motor.h"
#include "Robot.h"

std::vector<Motor *> Motor::allMotors;

void Motor::SetPositionAbsolute(double pos, int vel) {
    motor->move_absolute(pos, vel);
}

void Motor::SetPositionRelative(double pos, int vel) {
    motor->move_relative(pos, vel);
}

void Motor::SetVelocity(int16_t vel) {
    motor->move_velocity(vel);
}

void Motor::SetVoltage(int8_t voltage) {
    motor->move(voltage);
}

void Motor::SetPID(pros::motor_pid_s_t pid) {
    motor->set_pos_pid(pid);
    motor->set_vel_pid(pid);
}

pros::motor_pid_full_s_t Motor::GetPID() {
    return motor->get_vel_pid();
}

double Motor::GetVelocity() {
    return motor->get_actual_velocity();
}

double Motor::GetPosition() {
    return motor->get_position();
}