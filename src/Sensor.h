#pragma once

#include "api.h"
#include <vector>
#include "Vector.h"

typedef enum {
	INDEXER_FIRST = 1,
	INDEXER_SECOND = 3,
	ACCELEROMETER = 6,
	GYRO = 5,
	GYRO_2 = 7
} SensorID;

class Sensor {
public:

    static std::vector<Sensor *> allSensors;

    SensorID id;

    explicit Sensor(SensorID id);

    virtual std::int32_t GetValue() = 0;
};

class SensorButton : public Sensor {
private:
    pros::ADIDigitalIn prosButton;
public:
    explicit SensorButton(SensorID id);

    std::int32_t GetValue() override;
};

class Accelerometer : public Sensor {
private:
	pros::ADIAccelerometer prosAccelX;
	pros::ADIAccelerometer prosAccelY;
	pros::ADIAccelerometer prosAccelZ;

	Vec3 basisVector;

	double STEP;

public:
	explicit Accelerometer(SensorID id);

	std::int32_t GetValue() override;
	double GetX();
	double GetY();
	double GetZ();

	void Calibrate();

	Vec3 getRaw();
	Vec3 get();
};

class Gyro : public Sensor {
private:
	pros::ADIGyro prosGyro;

public:
	explicit Gyro(SensorID id);

	void Reset();

	std::int32_t GetValue() override;
};

class AnalogSensor : public Sensor {
private:
	pros::ADIAnalogIn sensor;
public:
	explicit AnalogSensor(SensorID id);

	std::int32_t GetValue() override;
};