#pragma once

#include <vector>
#include "Sensor.h"
#include "BotMotor.h"

namespace Robot {

    int GetState();

    pros::Vision GetCamera();

    BotMotor *GetMotor(BotMotorID id);

    Sensor *GetSensor(SensorID id);

    void Init();
};