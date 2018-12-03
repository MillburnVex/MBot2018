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
		Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(-127);//-std::clamp(newval, 100, 127));
    }
};

class IndexerComponent : public BotComponent {
	int target;
public:
	IndexerComponent() : BotComponent("Indexer Component",
		{
			ActionType::INDEXER_RUN
		}) {}

	void Execute(std::vector<ComponentAction> &actions) override {
		Robot::GetMotor(BotMotorID::INDEXER)->SetVoltage(Components::GetValue(actions, ActionType::INDEXER_RUN));
	}
};

class DriveComponent : public BotComponent {
	PID left = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
	PID right = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
	double latestLeft = 0;
	double latestRight = 0;
public:
    DriveComponent() : BotComponent("Drive component",
                                    {
											ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE, ActionType::DRIVE_TO, ActionType::ROTATE_TO
                                    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
		if (Components::IsActive(actions, ActionType::DRIVE_TO) || Components::IsActive(actions, ActionType::ROTATE_TO)) {
			int leftval = 0;
			int rightval = 0;
			if (Components::IsActive(actions, ActionType::DRIVE_TO)) {
				leftval = left.GetValue(
					Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetProsMotor()->get_position(),
					latestLeft + Components::GetValue(actions, ActionType::DRIVE_TO));
				rightval = -right.GetValue(
					-Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetProsMotor()->get_position(),
					-latestRight + Components::GetValue(actions, ActionType::DRIVE_TO));

			} else {
				leftval = left.GetValue(
					Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetProsMotor()->get_position(),
					latestLeft + Components::GetValue(actions, ActionType::ROTATE_TO));
				rightval = -right.GetValue(
					-Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetProsMotor()->get_position(),
					-latestRight - Components::GetValue(actions, ActionType::ROTATE_TO));
			}

			Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(std::clamp(leftval, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(std::clamp(leftval, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(std::clamp(rightval, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(std::clamp(rightval, -127, 127));
		} else {
			latestLeft  = Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetProsMotor()->get_position();
			latestRight = Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetProsMotor()->get_position();

			int linear = Components::GetValue(actions, ActionType::DRIVE_LINEAR);
			int rotate = Components::GetValue(actions, ActionType::DRIVE_ROTATE) * 0.7;
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(std::clamp(linear + rotate, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(std::clamp(linear + rotate, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(std::clamp(-linear + rotate, -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(std::clamp(-linear + rotate, -127, 127));
		}

        
    }
};

class BallLiftComponent : public BotComponent {
public:
    BallLiftComponent() : BotComponent("Ball lift component", {
            ActionType::BALL_LIFT_RUN
    }) {
        //pros::motor_pid_s ballLiftPID = pros::motor_pid_s();
        // jav do this
        //Robot::GetMotor(BotMotorID::BALL_LIFT)->vectorPID(ballLiftPID);
    }

    void Execute(std::vector<ComponentAction> &actions) override {
        Robot::GetMotor(BotMotorID::BALL_LIFT)->SetVoltage(Components::GetValue(actions, ActionType::BALL_LIFT_RUN));

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
    new BallLiftComponent();
	new IndexerComponent();
}

void Components::Execute(ActionType actionType) {
    Execute(actionType, 0);
}
