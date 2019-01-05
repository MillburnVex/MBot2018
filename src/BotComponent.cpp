#include <string>
#include <utility>
#include <algorithm>
#include "api.h"
#include "BotComponent.h"
#include "Robot.h"
#include "PID.h"

class FlywheelComponent : public BotComponent {
    PID pid = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
public:
    FlywheelComponent() : BotComponent("Flywheel component",
                                       {
                                               ActionType::FLYWHEEL_RUN
                                       }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
		Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(-127);
    }
};

class ArmComponent : public BotComponent {
	PID pid = PID(1.5f, 0.0f, 0.5f, 1000, -1000);
public:
	ArmComponent() : BotComponent("Arm component",
		{
			ActionType::ARM_SET
		}) {
		Robot::GetMotor(BotMotorID::ARM)->GetProsMotor()->tare_position();
	}

	void Execute(std::vector<ComponentAction> &actions) override {
		int voltage = pid.GetValue(
			Robot::GetMotor(BotMotorID::ARM)->GetPosition(),
			Components::GetValue(actions, ActionType::ARM_SET));
		Robot::GetMotor(BotMotorID::ARM)->SetVoltage(std::clamp(voltage, -127, 127));
	}
};

class IndexerComponent : public BotComponent {
	int target;
public:
	IndexerComponent() : BotComponent("Indexer component",
		{
			ActionType::INDEXER_RUN
		}) {}

	void Execute(std::vector<ComponentAction> &actions) override {
		Robot::GetMotor(BotMotorID::INDEXER)->SetVoltage(Components::GetValue(actions, ActionType::INDEXER_RUN));
	}
};

class DriveComponent : public BotComponent {
	PID leftFront = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
    PID leftBack = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
	PID rightFront = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
    PID rightBack = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
	double latestLeftFront = 0;
    double latestLeftBack = 0;
	double latestRightFront = 0;
    double latestRightBack = 0;
public:
    DriveComponent() : BotComponent("Drive component",
                                    {
											ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE, ActionType::DRIVE_TO, ActionType::ROTATE_TO
                                    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
		if (Components::IsActive(actions, ActionType::DRIVE_TO) || Components::IsActive(actions, ActionType::ROTATE_TO)) {
			int leftBackVal = 0;
            int leftFrontVal = 0;
            int rightBackVal = 0;
            int rightFrontVal = 0;
            /*
             * THIS CODE IS BAD. I know I shouldn't be copying and pasting this much, but honestly I think I've done goddamn
             * well enough on making my code idiomatic and so this is staying until it breaks.
             */
			if (Components::IsActive(actions, ActionType::DRIVE_TO)) {
				leftBackVal = leftBack.GetValue(
					Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->GetPosition(),
					latestLeftBack + Components::GetValue(actions, ActionType::DRIVE_TO));
                leftFrontVal = leftFront.GetValue(
                        Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetPosition(),
                        latestLeftFront + Components::GetValue(actions, ActionType::DRIVE_TO));
				rightBackVal = -rightBack.GetValue(
					-Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->GetPosition(),
					-latestRightBack + Components::GetValue(actions, ActionType::DRIVE_TO));
                rightFrontVal = -rightFront.GetValue(
                        -Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetPosition(),
                        -latestRightFront + Components::GetValue(actions, ActionType::DRIVE_TO));
			} else {
                leftBackVal = leftBack.GetValue(
                        Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->GetPosition(),
                        latestLeftBack + Components::GetValue(actions, ActionType::ROTATE_TO));
                leftFrontVal = leftFront.GetValue(
                        Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetPosition(),
                        latestLeftFront + Components::GetValue(actions, ActionType::ROTATE_TO));
                rightBackVal = -rightBack.GetValue(
                        -Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->GetPosition(),
                        -latestRightBack - Components::GetValue(actions, ActionType::ROTATE_TO));
                rightFrontVal = -rightFront.GetValue(
                        -Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetPosition(),
                        -latestRightFront - Components::GetValue(actions, ActionType::ROTATE_TO));
			}

			Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(std::clamp(leftFrontVal, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(std::clamp(leftBackVal, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(std::clamp(rightFrontVal, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(std::clamp(rightBackVal, -127, 127));
		} else {
			latestLeftFront  = Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetPosition();
			latestRightFront = Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetPosition();
            latestLeftBack  = Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->GetPosition();
            latestRightBack = Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->GetPosition();

			int linear = Components::GetValue(actions, ActionType::DRIVE_LINEAR);
			int rotate = Components::GetValue(actions, ActionType::DRIVE_ROTATE);
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(std::clamp(linear + rotate, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(std::clamp(linear + rotate, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(std::clamp(-linear + rotate, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(std::clamp(-linear + rotate, -127, 127));
		}

        
    }
};

class ReaperComponent : public BotComponent {
public:
    ReaperComponent() : BotComponent("Ball lift component", {
            ActionType::REAPER_RUN
    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
        Robot::GetMotor(BotMotorID::BALL_LIFT)->SetVoltage(Components::GetValue(actions, ActionType::REAPER_RUN));

    }
};

std::vector<BotComponent *> BotComponent::allComponents;
std::vector<ComponentAction> BotComponent::queue;

BotComponent::BotComponent(std::string name, std::vector<ActionType> validActions) :
        validActions(std::move(validActions)),
        name(std::move(name)) {
    BotComponent::allComponents.push_back(this);
}

bool Components::IsActive(std::vector<ComponentAction> &actions, ActionType actionType) {
    for (ComponentAction action : actions) {
        if (action.type == actionType) {
            return true;
        }
    }
    return false;
}

int Components::GetValue(std::vector<ComponentAction> &actions, ActionType actionType) {
    for (ComponentAction action : actions) {
        if (action.type == actionType) {
            return action.value;
        }
    }
    return ACTION_TYPE_NOT_ACTIVE;
}

void Components::Execute(ActionType actionType, int value) {
    ComponentAction action = {};
    action.value = value;
    action.type = actionType;
    BotComponent::queue.push_back(action);
}

void Components::Update() {
    std::vector<ComponentAction> actionsToSend;
    for (BotComponent *component : BotComponent::allComponents) {
        actionsToSend.clear();
        for (ComponentAction action : BotComponent::queue) {
            for (ActionType validActionType : component->validActions) {
                if (action.type == validActionType) {
                    actionsToSend.push_back(action);
                }
            }
        }
        component->Execute(actionsToSend);
    }
    BotComponent::queue.clear();
}

void Components::Init() {
    new DriveComponent();
    new FlywheelComponent();
    new ReaperComponent();
	new IndexerComponent();
	new ArmComponent();
}

void Components::Execute(ActionType actionType) {
    Execute(actionType, 0);
}
