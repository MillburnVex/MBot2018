#include "Sensor.h"
#include "Sensor.h"

std::vector<Sensor *> Sensor::allSensors;

const int32_t G = 295;

const double STEP = G / 9.81;

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

Accelerometer::Accelerometer(SensorID id) : Sensor(id), prosAccelX(pros::ADIAnalogIn(id)), prosAccelY(pros::ADIAnalogIn(id + 1)), prosAccelZ(pros::ADIAnalogIn(id + 2)) {

}

std::int32_t Accelerometer::GetValue()
{
	return std::int32_t();
}

double Accelerometer::GetX() {
	return (prosAccelX.get_value()-2048)/STEP;
}

double Accelerometer::GetY() {
	return (prosAccelY.get_value()-2048)/STEP;
}

double Accelerometer::GetZ() {
	return (prosAccelZ.get_value()-2048)/STEP;
}


Gyro::Gyro(SensorID id) : Sensor(id), prosGyro(pros::ADIGyro(id, 1)) {

}

int32_t Gyro::GetValue() {
	return prosGyro.get_value();
}