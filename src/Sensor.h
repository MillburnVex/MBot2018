#pragma once
#include "api.h"
#include <vector>

typedef enum {
    BUTTON_BALL_LIFT_UPPER
} SensorID;

class Sensor {
public:

    static std::vector<Sensor*> allSensors;

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