#include <string>
#include <utility>
#include "api.h"
#include "BotComponent.h"
#include "Robot.h"

class FlywheelComponent : public BotComponent {
public:
FlywheelComponent() : BotComponent("Flywheel component",
	{
		ComponentAction::FLYWHEEL_FULL_SPEED, ComponentAction::FLYWHEEL_STOP
	}) {}

void Execute(ComponentAction action, int value) override {
	if (action == ComponentAction::FLYWHEEL_FULL_SPEED) {
		Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(127);
	} else if (action == ComponentAction::FLYWHEEL_STOP) {
		Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(0);
	}
}
};

class DriveComponent : public BotComponent {
public:
DriveComponent() : BotComponent("Drive component",
	{
		ComponentAction::DRIVE_LINEAR, ComponentAction::DRIVE_ROTATE
	}) {}

void Execute(ComponentAction action, int value) override {
	if (action == ComponentAction::DRIVE_LINEAR) {
		Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(value);
		Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(value);
		Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(value);
		Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(value);
	} else if(action == ComponentAction::DRIVE_ROTATE) {
		Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(value);
		Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(value);
		Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(-value);
		Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(-value);
	}
}
};

class BallLiftComponent : public BotComponent {
public:
	BallLiftComponent() : BotComponent("Ball lift component", {
		ComponentAction::BALL_LIFT_DOWN, ComponentAction::BALL_LIFT_STOP, ComponentAction::BALL_LIFT_UP
		}) {}

	void Execute(ComponentAction action, int value) override {
		if (action == ComponentAction::BALL_LIFT_DOWN) {

		}
	}
};

std::vector<BotComponent *> BotComponent::allComponents;

BotComponent::BotComponent(std::string name, std::vector<ComponentAction> validActions) :
	validActions(std::move(validActions)),
	name(std::move(name)) {
	BotComponent::allComponents.push_back(this);
}

void Components::Execute(ComponentAction action, int value) {
	for (auto component : BotComponent::allComponents) {
		for (auto validAction : component->validActions) {
			if (validAction == action) {
				component->Execute(action, value);
			}
		}
	}
}

void Components::Init() {
	new DriveComponent();
	new FlywheelComponent();
}
