#pragma once

#include <vector>
#include "BotMotorConfig.h"
#include "BotMotor.h"

namespace Robot {

    int GetState();

	pros::Vision GetCamera();

    BotMotor *GetMotor(BotMotorID id);

    void Init();
};