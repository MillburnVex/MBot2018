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
    virtual void SetVoltage(int8_t voltage) = 0;

    /**
     * Sets the velocity of the motor
     * @param vel
     */
    virtual void SetVelocity(int8_t vel) = 0;

    /**
     * Sets the position goal of the motor in absolute degrees relative to the last time it was recalibrated
     * @param pos the position goal in degrees
     * @param vel the maximum allowable velocity
     */
    virtual void SetPositionAbsolute(double pos, int vel) = 0;

    /**
     * Sets the position goal of the motor in degrees relative to its current position
     * @param pos the amount to move in degrees
     * @param vel the maximum allowable velocity
     */
    virtual void SetPositionRelative(double pos, int vel) = 0;

    /**
     *
     * @param pid
     */
    virtual void SetPID(pros::motor_pid_s_t pid) = 0;

    virtual pros::motor_pid_full_s_t GetPID() = 0;

    virtual bool ContainsRealMotor() = 0;

    virtual pros::Motor GetProsMotor() = 0;

    BotMotorConfig GetConfig() { return config; };
};

class RealMotor final : public BotMotor {
private:
    pros::Motor *motor;

public:
    explicit RealMotor(std::uint8_t id) : RealMotor(BotMotorConfig(id)) {};

    explicit RealMotor(BotMotorConfig config) : BotMotor(config) {
        motor = new pros::Motor(config.id, config.gearset, config.reverse, pros::E_MOTOR_ENCODER_DEGREES);
    };

    void SetVoltage(int8_t voltage) override;

    void SetVelocity(int8_t vel) override;

    void SetPositionAbsolute(double pos, int vel) override;

    void SetPositionRelative(double pos, int vel) override;

    void SetPID(pros::motor_pid_s_t pid) override;

    pros::motor_pid_full_s_t GetPID() override;

    bool ContainsRealMotor() override { return true; }

    pros::Motor GetProsMotor() override { return *motor; }
};
