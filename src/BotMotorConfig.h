#pragma once

#include <cstdint>
#include "api.h"

/**
 * A class that contains information and settings about a motor
 */
struct BotMotorConfig {
    /**
     * The identifier of the motor
     */
    std::uint8_t id;
    /**
     * If true, this motor will run in reverse, otherwise it will run normally
     */
    bool reverse;
    /**
     * The PROS motor gearset to use. Options are pros::E_MOTOR_GEARSET_<06|18|36>
     */
    pros::motor_gearset_e gearset;

    explicit BotMotorConfig(std::uint8_t id) : BotMotorConfig(id, false, pros::E_MOTOR_GEARSET_06) {};

    BotMotorConfig(std::uint8_t id, bool reverse, pros::motor_gearset_e_t gearset) : id(id), reverse(reverse),
                                                                                     gearset(gearset) {};
};