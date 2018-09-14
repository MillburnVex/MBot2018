#include "../include/main.h"

#pragma once

class BotMotorConfig
{
public:
	std::uint8_t id;
	bool reverse;
	pros::motor_gearset_e gearset;

	BotMotorConfig(std::uint8_t id) : BotMotorConfig(id, false, pros::E_MOTOR_GEARSET_06) {};
	BotMotorConfig(std::uint8_t id, bool reverse, pros::motor_gearset_e_t gearset);
};

class BotMotor
{
private:
	BotMotorConfig cfg;
public:
	BotMotor(std::uint8_t id) : BotMotor(BotMotorConfig(id)){};
	BotMotor(BotMotorConfig cfg);

	virtual void set(int vel) = 0;
	virtual void setVoltage(int voltage) = 0;
	virtual void setVelocity(int vel) = 0;
	virtual void setPosition(double pos, int vel) = 0;
	virtual void setPositionRelative(double pos, int vel) = 0;
	virtual void setPID(pros::motor_pid_s_t pid) = 0;

	virtual pros::motor_pid_full_s_t getPID() = 0;
	virtual bool containsRealMotor() = 0;
	virtual pros::Motor getRealMotor() = 0;

	BotMotorConfig getConfig() { return cfg; };
};

class RealMotor : public BotMotor
{
private:
	pros::Motor *motor;

public:
	RealMotor(std::uint8_t id) : RealMotor(BotMotorConfig(id)) {};
	RealMotor(BotMotorConfig cfg) : BotMotor(cfg)
	{
		motor = new pros::Motor(cfg.id, cfg.gearset, cfg.reverse, pros::E_MOTOR_ENCODER_DEGREES);
	};

	void set(int vel);
	void setVoltage(int voltage);
	void setVelocity(int vel);
	void setPosition(double pos, int vel);
	void setPositionRelative(double pos, int vel);
	void setPID(pros::motor_pid_s_t pid);

	pros::motor_pid_full_s_t getPID();
	bool containsRealMotor() { return true; }
	pros::Motor getRealMotor() { return *motor; }

};
