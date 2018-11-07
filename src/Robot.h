#pragma once

#include <vector>
#include "Sensor.h"
#include "BotMotor.h"

namespace Robot {

    int GetState();

    BotMotor *GetMotor(BotMotorID id);

    Sensor *GetSensor(SensorID id);

    void Init();
};