#include <string>
#include <utility>
#include <algorithm>
#include "api.h"
#include "BotComponent.h"
#include "Robot.h"
#include "PID.h"

class FlywheelComponent : public BotComponent {
	PID pid = PID(1, 0.0f, 1, 1000, -1000);
public:
    FlywheelComponent() : BotComponent("Flywheel component",
                                       {
                                               ActionType::FLYWHEEL_RUN
                                       }) {}
	 
    void Execute(std::vector<ComponentAction> &actions) override {
		//printf("%f\n", -Robot::GetMotor(BotMotorID::FLYWHEEL)->GetProsMotor()->get_actual_velocity());
		auto newval = pid.GetValue(-Robot::GetMotor(BotMotorID::FLYWHEEL)->GetProsMotor()->get_actual_velocity(), Components::GetValue(actions, ActionType::FLYWHEEL_RUN));
		//printf("pid value %d\n", newval);
		//printf("clamped %d\n", std::clamp(newval, 0, 127));
		Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(-std::clamp(newval, 100, 127));
        
    }
};

class IndexerComponent : public BotComponent {
	PID pid = PID(1, 0.0f, 1, 1000, -1000);
	int target;
public:
	IndexerComponent() : BotComponent("Indexer Component",
		{
			ActionType::INDEXER_UP, ActionType::INDEXER_DOWN
		}) {}

	void Execute(std::vector<ComponentAction> &actions) override {
		if (Components::IsActive(actions, ActionType::INDEXER_DOWN)) target = 0;
		if (Components::IsActive(actions, ActionType::INDEXER_UP)) target = 45;

		auto newval = pid.GetValue(Robot::GetMotor(BotMotorID::INDEXER)->GetProsMotor()->get_position(), target);
		Robot::GetMotor(BotMotorID::INDEXER)->SetVoltage(std::clamp(newval, -127, 127));
	}
};

class DriveComponent : public BotComponent {
public:
    DriveComponent() : BotComponent("Drive component",
                                    {
                                            ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE
                                    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
        int linear = Components::GetValue(actions, ActionType::DRIVE_LINEAR);
        int rotate = Components::GetValue(actions, ActionType::DRIVE_ROTATE) * 0.7;
		printf("linear: %d\n", linear);
		printf("rotate: %d\n", rotate);
		Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVelocity(std::clamp(linear+rotate, -200, 200));
		Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVelocity(std::clamp(linear+rotate, -200, 200));
		Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVelocity(std::clamp(-linear+rotate, -200, 200));
		Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVelocity(std::clamp(-linear+rotate, -200, 200));

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
		//printf("VICTORY ROYALE %d\n");
		int speed = Components::GetValue(actions, ActionType::BALL_LIFT_RUN);
        Robot::GetMotor(BotMotorID::BALL_LIFT)->SetVoltage(speed);
      
    }
};

class CapLiftComponent : public BotComponent {
	PID pid = PID(1, 0.0f, 1, 1000, -1000);
	int lastval = 0;
public:
    CapLiftComponent() : BotComponent("Cap lift component", {
            ActionType::CAP_LIFT_DOWN, ActionType::CAP_LIFT_HOLD, ActionType::CAP_LIFT_UP
    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
        if(Components::IsActive(actions, ActionType::CAP_LIFT_UP)) {
			lastval = Robot::GetMotor(BotMotorID::CAP_LIFT)->GetProsMotor()->get_position();
			Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVoltage(127);
        }
		else if(Components::IsActive(actions, ActionType::CAP_LIFT_HOLD)) {
			auto newval = pid.GetValue(Robot::GetMotor(BotMotorID::CAP_LIFT)->GetProsMotor()->get_position(), lastval);
			printf("newval %d\n", newval);
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVoltage(std::clamp(newval, -127, 127));
        }
		else if(Components::IsActive(actions, ActionType::CAP_LIFT_DOWN)) {
			lastval = Robot::GetMotor(BotMotorID::CAP_LIFT)->GetProsMotor()->get_position();
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVoltage(-127);
        }
    }
};

class ClawComponent : public BotComponent {
	PID pid = PID(1, 0.0f, 1, 1000, -1000);
	int lastval = 0;
public:
    ClawComponent() : BotComponent("Claw component", {
            ActionType::CLAW_FOLD_UP, ActionType::CLAW_FOLD_DOWN
    }) {
        //pros::motor_pid_s clawPID = pros::motor_pid_s();
        // jav do this
        //Robot::GetMotor(BotMotorID::CLAW)->vectorPID(clawPID);
    }

    void Execute(std::vector<ComponentAction>& actions) override {
		if (Components::IsActive(actions, ActionType::CLAW_FOLD_UP)) {
			lastval = Robot::GetMotor(BotMotorID::CLAW)->GetProsMotor()->get_position();
			Robot::GetMotor(BotMotorID::CLAW)->SetVoltage(100);
		}
		else if (Components::IsActive(actions, ActionType::CLAW_FOLD_DOWN)) {
			lastval = Robot::GetMotor(BotMotorID::CLAW)->GetProsMotor()->get_position();
			Robot::GetMotor(BotMotorID::CLAW)->SetVoltage(-100);
		}
		else {
			auto newval = pid.GetValue(Robot::GetMotor(BotMotorID::CLAW)->GetProsMotor()->get_position(), lastval);
			Robot::GetMotor(BotMotorID::CLAW)->SetVoltage(std::clamp(newval, -127, 127));
		}
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
    new CapLiftComponent();
    new ClawComponent();
	new IndexerComponent();
}

void Components::Execute(ActionType actionType) {
    Execute(actionType, 0);
}
