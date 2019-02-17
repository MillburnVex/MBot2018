#pragma once

#include <vector>
#include "Sensor.h"
#include "Motor.h"

typedef enum {
	RED, BLUE
} Team;
typedef enum {
	FRONT_PARK,
	FRONT_NO_PARK_CAP_FLIP, FRONT_NO_PARK_DOUBLE_SCRAPE_MIDDLE, FRONT_NO_PARK_DOUBLE_SCRAPE_DESCORE,
	BACK_PARK_MIDDLE, BACK_PARK_DESCORE, BACK_PARK_TWO_CAP, BACK_NO_PARK_MIDDLE_CAP_FLIP, BACK_NO_PARK_DESCORE_CAP_FLIP, BACK_NO_PARK_TWO_CAP
} Strategy;
namespace Robot {

	int GetUpdateMillis();

	void SetTeam(Team team);

	void SetAutonStrategy(Strategy strat);

	void SetDriveBrakeMode(pros::motor_brake_mode_e_t mode);

	Team GetTeam();

	Strategy GetAutonStrategy();

    pros::Vision GetCamera();

    Motor *GetMotor(MotorID id);

    Sensor *GetSensor(SensorID id);

    bool BallInFirstZone();

    bool BallInSecondZone();

    bool BallLoaded();

	int GetRotation();

	void ResetRotation(int offset = 0);

	bool IsInManualMode();

    void Init();

    pros::Controller GetMasterController();

	pros::Controller GetPartnerController();
};