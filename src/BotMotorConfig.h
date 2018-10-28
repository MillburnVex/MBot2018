#pragma once

#include <cstdint>
#include "api.h"

typedef enum {
    FLYWHEEL,
    BALL_LIFT,
    DRIVE_LEFT_FRONT, DRIVE_LEFT_BACK, DRIVE_RIGHT_FRONT, DRIVE_RIGHT_BACK,
    CAP_LIFT,
    CLAW
} BotMotorID;

/**
 * A class that contains information and settings about a motor
 */
struct BotMotorConfig {
    /**
     * The identifier of the motor
     */
    BotMotorID id;
    /**
     * If true, this motor will run in reverse, otherwise it will run normally
     */
    bool reverse;
    /**
     * The PROS motor gearset to use. Options are pros::E_MOTOR_GEARSET_<06|18|36>
     */
    pros::motor_gearset_e gearset;

    explicit BotMotorConfig(BotMotorID id) : BotMotorConfig(id, false, pros::E_MOTOR_GEARSET_06) {};

    BotMotorConfig(BotMotorID id, bool reverse, pros::motor_gearset_e_t gearset) : id(id), reverse(reverse),
                                                                                     gearset(gearset) {};
};