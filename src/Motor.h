#pragma once

#include <vector>
#include "MotorConfig.h"

class Motor {
private:
    MotorConfig config;
    pros::Motor *motor;

public:
    static std::vector<Motor *> allMotors;

    explicit Motor(MotorConfig config) : config(config) {
        motor = new pros::Motor(config.id, config.gearset, config.reverse, pros::E_MOTOR_ENCODER_DEGREES);
        allMotors.push_back(this);
    };

    explicit Motor(MotorID id) : Motor(MotorConfig(id)) {};

    /**
     * Sets the voltage of the motor
     * @param voltage an int from -127 to 127
     */
    void SetVoltage(int8_t voltage);

    /**
     * Sets the velocity of the motor
     * @param vel
     */
    void SetVelocity(int16_t vel);

    /**
     * Sets the position goal of the motor in absolute degrees relative to the last time it was recalibrated
     * @param pos the position goal in degrees
     * @param vel the maximum allowable velocity
     */
    void SetPositionAbsolute(double pos, int vel);

    /**
     * Sets the position goal of the motor in degrees relative to its current position
     * @param pos the amount to move in degrees
     * @param vel the maximum allowable velocity
     */
    void SetPositionRelative(double pos, int vel);

    void SetPID(pros::motor_pid_s_t pid);

    double GetPosition();

    double GetVelocity();

    pros::motor_pid_full_s_t GetPID();

    pros::Motor *GetProsMotor() {
        return motor;
    }

    MotorConfig GetConfig() {
        return config;
    };
};