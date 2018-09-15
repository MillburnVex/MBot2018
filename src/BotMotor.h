#include "../include/main.h"
#include "BotMotorConfig.h"

#pragma once

class BotMotor {
private:
    BotMotorConfig config;
public:
    explicit BotMotor(std::uint8_t id) : BotMotor(BotMotorConfig(id)) {};

    explicit BotMotor(BotMotorConfig config) : config(config) {};

    /**
     * Sets the voltage of the motor
     * @param voltage an int from -127 to 127
     */
    virtual void setVoltage(int8_t voltage) = 0;

    /**
     * Sets the velocity of the motor
     * @param vel
     */
    virtual void setVelocity(int8_t vel) = 0;

    /**
     * Sets the position goal of the motor in absolute degrees relative to the last time it was recalibrated
     * @param pos the position goal in degrees
     * @param vel the maximum allowable velocity
     */
    virtual void setPositionAbsolute(double pos, int vel) = 0;

    /**
     * Sets the position goal of the motor in degrees relative to its current position
     * @param pos the amount to move in degrees
     * @param vel the maximum allowable velocity
     */
    virtual void setPositionRelative(double pos, int vel) = 0;

    /**
     *
     * @param pid
     */
    virtual void setPID(pros::motor_pid_s_t pid) = 0;

    virtual pros::motor_pid_full_s_t getPID() = 0;

    virtual bool containsRealMotor() = 0;

    virtual pros::Motor getProsMotor() = 0;

    BotMotorConfig getConfig() { return config; };
};

class RealMotor final : public BotMotor {
private:
    pros::Motor *motor;

public:
    explicit RealMotor(std::uint8_t id) : RealMotor(BotMotorConfig(id)) {};

    explicit RealMotor(BotMotorConfig config) : BotMotor(config) {
        motor = new pros::Motor(config.id, config.gearset, config.reverse, pros::E_MOTOR_ENCODER_DEGREES);
    };

    void setVoltage(int8_t voltage) override;

    void setVelocity(int8_t vel) override;

    void setPositionAbsolute(double pos, int vel) override;

    void setPositionRelative(double pos, int vel) override;

    void setPID(pros::motor_pid_s_t pid) override;

    pros::motor_pid_full_s_t getPID() override;

    bool containsRealMotor() override { return true; }

    pros::Motor getProsMotor() override { return *motor; }
};
