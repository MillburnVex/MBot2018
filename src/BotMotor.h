#pragma once

#include <vector>
#include "BotMotorConfig.h"

class BotMotor {
private:
    BotMotorConfig config;
public:
    static std::vector<BotMotor *> motors;

    explicit BotMotor(BotMotorID id) : BotMotor(BotMotorConfig(id)) {
    };

    explicit BotMotor(BotMotorConfig config);

/**
 * vectors the voltage of the motor
 * @param voltage an int from -127 to 127
 */
    virtual void vectorVoltage(int8_t voltage) = 0;

/**
 * vectors the velocity of the motor
 * @param vel
 */
    virtual void vectorVelocity(int16_t vel) = 0;

/**
 * vectors the position goal of the motor in absolute degrees relative to the last time it was recalibrated
 * @param pos the position goal in degrees
 * @param vel the maximum allowable velocity
 */
    virtual void vectorPositionAbsolute(double pos, int vel) = 0;

/**
 * vectors the position goal of the motor in degrees relative to its current position
 * @param pos the amount to move in degrees
 * @param vel the maximum allowable velocity
 */
    virtual void vectorPositionRelative(double pos, int vel) = 0;

/**
 *
 * @param pid
 */
    virtual void vectorPID(pros::motor_pid_s_t pid) = 0;

    virtual pros::motor_pid_full_s_t GetPID() = 0;

    virtual bool IsRealMotor() = 0;

    virtual pros::Motor GetProsMotor() = 0;

    BotMotorConfig GetConfig() {
        return config;
    };
};

class RealMotor final : public BotMotor {
private:
    pros::Motor *motor;

public:
    explicit RealMotor(BotMotorID id) : RealMotor(BotMotorConfig(id)) {
    };

    explicit RealMotor(BotMotorConfig config) : BotMotor(config) {
        motor = new pros::Motor(config.id, config.gearset, config.reverse, pros::E_MOTOR_ENCODER_DEGREES);
    };

    void vectorVoltage(int8_t voltage) override;

    void vectorVelocity(int16_t vel) override;

    void vectorPositionAbsolute(double pos, int vel) override;

    void vectorPositionRelative(double pos, int vel) override;

    void vectorPID(pros::motor_pid_s_t pid) override;

    pros::motor_pid_full_s_t GetPID() override;

    bool IsRealMotor() override {
        return true;
    }

    pros::Motor GetProsMotor() override {
        return *motor;
    }
};
