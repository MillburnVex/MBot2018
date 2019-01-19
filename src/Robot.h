#pragma once

#include <vector>
#include "Sensor.h"
#include "Motor.h"

typedef enum {
	RED, BLUE
} Team;
typedef enum {
	FRONT, BACK
} Position;

namespace Robot {

	int GetUpdateMillis();

	void SetTeam(Team team);

	void SetAutonPosition(Position pos);

	void SetDriveBrakeMode(pros::motor_brake_mode_e_t mode);

	Team GetTeam();

	Position GetAutonPosition();

    int GetState();

    pros::Vision GetCamera();

    Motor *GetMotor(MotorID id);

    Sensor *GetSensor(SensorID id);

	bool IsInManualMode();

    void Init();
};