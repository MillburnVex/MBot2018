#pragma once

#include <vector>
#include "BotMotorConfig.h"
#include "BotMotor.h"

namespace Robot {

    int GetState();

    BotMotor *GetMotor(BotMotorID id);

    void Init();
};