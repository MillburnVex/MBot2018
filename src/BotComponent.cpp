#include <string>
#include <utility>
#include <algorithm>
#include "api.h"
#include "BotComponent.h"
#include "Robot.h"
#include "PID.h"

class FlywheelComponent : public BotComponent {
	PID pid = PID(1, 1, 1, 1000, -1000);
public:
    FlywheelComponent() : BotComponent("Flywheel component",
                                       {
                                               ActionType::FLYWHEEL_RUN
                                       }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
		printf("df\n");
		auto newval = pid.GetValue(Robot::GetMotor(BotMotorID::FLYWHEEL)->GetProsMotor()->get_actual_velocity(), Components::GetValue(actions, ActionType::FLYWHEEL_RUN));
		printf("%d\n", newval);
		Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(fmax(0, newval));
        
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
public:
    CapLiftComponent() : BotComponent("Cap lift component", {
            ActionType::CAP_LIFT_DOWN, ActionType::CAP_LIFT_DROP, ActionType::CAP_LIFT_HOLD, ActionType::CAP_LIFT_UP
    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
        if(Components::IsActive(actions, ActionType::CAP_LIFT_UP)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVelocity(8);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_HOLD)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVelocity(0);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_DROP)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVelocity(0);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_DOWN)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVelocity(-8);
        }
    }
};

class ClawComponent : public BotComponent {
public:
    ClawComponent() : BotComponent("Claw component", {
            ActionType::CLAW_FOLD_UP, ActionType::CLAW_FOLD_DOWN
    }) {
        //pros::motor_pid_s clawPID = pros::motor_pid_s();
        // jav do this
        //Robot::GetMotor(BotMotorID::CLAW)->vectorPID(clawPID);
    }

    void Execute(std::vector<ComponentAction>& actions) override {
		printf("started\n");
        if(Components::IsActive(actions, ActionType::CLAW_FOLD_DOWN)) {
            // this needs testing
			Robot::GetMotor(BotMotorID::CLAW)->SetVoltage(-100);
		}
		else if (Components::IsActive(actions, ActionType::CLAW_FOLD_UP)) {
			Robot::GetMotor(BotMotorID::CLAW)->SetVoltage(100);
		}
		else {
			Robot::GetMotor(BotMotorID::CLAW)->SetVoltage(0);
		}
		printf("ended\n");
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
}

void Components::Execute(ActionType actionType) {
    Execute(actionType, 0);
}
