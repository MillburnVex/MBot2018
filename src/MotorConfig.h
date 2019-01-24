#pragma once

#include <cstdint>
#include "api.h"

typedef enum {
	FLYWHEEL = 5,
	BALL_LIFT = 13,
	DRIVE_LEFT_FRONT = 1, DRIVE_LEFT_BACK = 2, DRIVE_RIGHT_FRONT = 21, DRIVE_RIGHT_BACK = 9,
	ARM = 12,
	INDEXER = 8
} MotorID;

/**
 * A class that contains information and settings about a motor
 */
struct MotorConfig {
    /**
     * The identifier of the motor
     */
    MotorID id;
    /**
     * If true, this motor will run in reverse, otherwise it will run normally
     */
    bool reverse;
    /**
     * The PROS motor gearset to use. Options are pros::E_MOTOR_GEARSET_<06|18|36>
     */
    pros::motor_gearset_e gearset;

    explicit MotorConfig(MotorID id) : MotorConfig(id, false, pros::E_MOTOR_GEARSET_18) {};

    MotorConfig(MotorID id, bool reverse, pros::motor_gearset_e_t gearset) : id(id), reverse(reverse),
                                                                                   gearset(gearset) {};
};