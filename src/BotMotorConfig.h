#pragma once

#include <cstdint>
#include "api.h"

typedef enum {
	FLYWHEEL = 5,
	BALL_LIFT = 3,
	DRIVE_LEFT_FRONT = 1, DRIVE_LEFT_BACK = 2, DRIVE_RIGHT_FRONT = 10, DRIVE_RIGHT_BACK = 9,
	INDEXER = 8

} BotMotorID;

/**
 * A class that contains information and vectortings about a motor
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
     * The PROS motor gearset to use. Options are pros::E_MOTOR_gearset_<06|18|36>
     */
    pros::motor_gearset_e gearset;

    explicit BotMotorConfig(BotMotorID id) : BotMotorConfig(id, false, pros::E_MOTOR_GEARSET_18) {};

    BotMotorConfig(BotMotorID id, bool reverse, pros::motor_gearset_e_t gearset) : id(id), reverse(reverse),
                                                                                   gearset(gearset) {};
};