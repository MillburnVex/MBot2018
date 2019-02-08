#include "Sensor.h"
#include "Sensor.h"
#include "Sensor.h"
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

Accelerometer::Accelerometer(SensorID id) : Sensor(id), prosAccelX(pros::ADIAnalogIn(id)), prosAccelY(pros::ADIAnalogIn(id + 1)), prosAccelZ(pros::ADIAnalogIn(id + 2)) {
	
}

void Accelerometer::Calibrate() {
	auto realX = prosAccelX.get_value();
	auto realY = prosAccelY.get_value();
	auto realZ = prosAccelZ.get_value();

	basisVector = Vec3(realX, realY, realZ);

	auto stepVector = basisVector - 2048;
	STEP = int(stepVector.length()) / 9.81;

	printf("step: %f, %f, %f\n", stepVector.x(), stepVector.y(), stepVector.z());
}

std::int32_t Accelerometer::GetValue()
{
	return std::int32_t();
}

double Accelerometer::GetX() {
	return get().x();
}

double Accelerometer::GetY() {
	return get().y();
}

double Accelerometer::GetZ() {
	return get().z();
}

Vec3 Accelerometer::getRaw()
{
	return Vec3(prosAccelX.get_value(), prosAccelY.get_value(), prosAccelZ.get_value());
}

Vec3 Accelerometer::get()
{
	return (getRaw() - basisVector)/STEP;
}


Gyro::Gyro(SensorID id) : Sensor(id), prosGyro(pros::ADIGyro(id, 1)) {

}

int32_t Gyro::GetValue() {
	return prosGyro.get_value();
}