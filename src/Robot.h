#pragma once

#include <vector>
#include "Sensor.h"
#include "BotMotor.h"

typedef enum {
	RED, BLUE
} Team;
typedef enum {
	FRONT, BACK
} Position;

namespace Robot {

	void SetTeam(Team team);

	void SetAutonPosition(Position pos);

	Team GetTeam();

	Position GetAutonPosition();

    int GetState();

    pros::Vision GetCamera();

    BotMotor *GetMotor(BotMotorID id);

    Sensor *GetSensor(SensorID id);

    void Init();
};