#pragma once

#include "api.h"
#include <vector>

typedef enum {
	INDEXER_BUTTON = 1,
	INDEXER_BUTTON2 = 3
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

class AnalogSensor : public Sensor {
private:
	pros::ADIAnalogIn sensor;
public:
	explicit AnalogSensor(SensorID id);

	std::int32_t GetValue() override;
};