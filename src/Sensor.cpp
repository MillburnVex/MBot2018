#include "Sensor.h"

std::vector<Sensor *> Sensor::allSensors;

Sensor::Sensor(SensorID id) : id(id) {
    allSensors.push_back(this);
}

SensorButton::SensorButton(SensorID id) : Sensor(id), prosButton(pros::ADIDigitalIn(id)) {
}

int32_t SensorButton::GetValue() {
	return prosButton.get_value();
}

AnalogSensor::AnalogSensor(SensorID id) : Sensor(id), sensor(pros::ADIAnalogIn(id)) {
}

int32_t AnalogSensor::GetValue() {
	return sensor.get_value();
}
