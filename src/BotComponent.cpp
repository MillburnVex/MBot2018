#include <vector>
#include <functional>
#include "ComponentAction.cpp"

namespace Components {
	std::vector<BotComponent> allComponents{};
}

class BotComponent {
public:
    std::vector<ComponentAction> validActions;
	std::vector<ComponentAction> activeActions{};

	BotComponent(std::vector<ComponentAction> validActions) : 
		validActions(validActions) {
		allComponents.push_back(this);
	}

	bool IsActive(ComponentAction action) {
		for (auto a : activeActions) {
			if (a == action) {
				return true;
			}
		}
		return false;
	}

    virtual void Execute(ComponentAction action, int value) {
		
    }

    virtual void Update() {
    }
};

class FlywheelComponent : public BotComponent {
public:
	 FlywheelComponent() : BotComponent({ ComponentAction::FLYWHEEL_FULL_SPEED, ComponentAction::FLYWHEEL_STOP }) {}

	 void Execute(ComponentAction action, int value) override {
		 if (action == ComponentAction::FLYWHEEL_FULL_SPEED) {

		 } else if (action == ComponentAction::FLYWHEEL_STOP) {
			  
		 }
	 }

	 void Update() override {
		 if (IsActive(ComponentAction::FLYWHEEL_FULL_SPEED)) {
			 // motor to full power
		 } else if (IsActive(ComponentAction::FLYWHEEL_STOP)) {
			 // motor to 0 power
		 }
	 }
};

class DriveComponent : public BotComponent {
public:
	DriveComponent() : BotComponent({ ComponentAction::DRIVE_LINEAR, ComponentAction::DRIVE_ROTATE }) {}
};