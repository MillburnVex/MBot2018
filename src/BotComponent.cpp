#include <string>
#include <utility>
#include "api.h"
#include "BotComponent.h"
#include "Robot.h"

class FlywheelComponent : public BotComponent {
public:
    FlywheelComponent() : BotComponent("Flywheel component",
                                       {
                                               ActionType::FLYWHEEL_FULL_SPEED, ActionType::FLYWHEEL_STOP,
                                               ActionType::FLYWHEEL_PASSIVE_SPEED
                                       }) {}

    void Execute(std::vector<ComponentAction *> &actions) override {
        if (Components::IsActive(actions, ActionType::FLYWHEEL_FULL_SPEED)) {
            Robot::GetMotor(BotMotorID::FLYWHEEL)->vectorVoltage(127);
        } else if (Components::IsActive(actions, ActionType::FLYWHEEL_STOP)) {
            Robot::GetMotor(BotMotorID::FLYWHEEL)->vectorVoltage(0);
        } else if (Components::IsActive(actions, ActionType::FLYWHEEL_PASSIVE_SPEED)) {
            Robot::GetMotor(BotMotorID::FLYWHEEL)->vectorVoltage(80);
        }
    }
};

class DriveComponent : public BotComponent {
public:
    DriveComponent() : BotComponent("Drive component",
                                    {
                                            ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE
                                    }) {}

    void Execute(std::vector<ComponentAction *> &actions) override {
        int linear = Components::GetValue(actions, ActionType::DRIVE_LINEAR);
        int rotate = Components::GetValue(actions, ActionType::DRIVE_ROTATE);
        if (linear != ACTION_TYPE_NOT_ACTIVE && rotate != ACTION_TYPE_NOT_ACTIVE) {
            // linear and rotate at the same time! yay
        } else if (linear != ACTION_TYPE_NOT_ACTIVE) {
            // pretty sure just vectorting voltage is fine, cuz the value and the voltage have a domain/range of [-127, 127]
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->vectorVoltage(linear);
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->vectorVoltage(linear);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->vectorVoltage(linear);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->vectorVoltage(linear);
        } else if (rotate != ACTION_TYPE_NOT_ACTIVE) {
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->vectorVoltage(rotate);
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->vectorVoltage(rotate);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->vectorVoltage(-rotate);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->vectorVoltage(-rotate);
        }
    }
};

class BallLiftComponent : public BotComponent {
public:
    BallLiftComponent() : BotComponent("Ball lift component", {
            ActionType::BALL_LIFT_DOWN, ActionType::BALL_LIFT_STOP, ActionType::BALL_LIFT_UP
    }) {
        //pros::motor_pid_s ballLiftPID = pros::motor_pid_s();
        // jav do this
        //Robot::GetMotor(BotMotorID::BALL_LIFT)->vectorPID(ballLiftPID);
    }

    void Execute(std::vector<ComponentAction *> &actions) override {
        if (Components::IsActive(actions, ActionType::BALL_LIFT_DOWN)) {
            // needs tuning. I figure lower is better for now
            Robot::GetMotor(BotMotorID::BALL_LIFT)->vectorVelocity(8);
        } else if (Components::IsActive(actions, ActionType::BALL_LIFT_UP)) {
			Robot::GetMotor(BotMotorID::BALL_LIFT)->vectorVelocity(-1000);
			printf("ds\n");
            
        }
    }
};

class CapLiftComponent : public BotComponent {
public:
    CapLiftComponent() : BotComponent("Cap lift component", {
            ActionType::CAP_LIFT_DOWN, ActionType::CAP_LIFT_DROP, ActionType::CAP_LIFT_HOLD, ActionType::CAP_LIFT_UP
    }) {}

    void Execute(std::vector<ComponentAction *> &actions) override {
        if(Components::IsActive(actions, ActionType::CAP_LIFT_UP)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->vectorVelocity(8);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_HOLD)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->vectorVelocity(0);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_DROP)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->vectorVoltage(0);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_DOWN)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->vectorVelocity(-8);
        }
    }
};

class ClawComponent : public BotComponent {
public:
    ClawComponent() : BotComponent("Claw component", {
            ActionType::CLAW_FOLD_UP, ActionType::CLAW_FOLD_DOWN
    }) {
        pros::motor_pid_s clawPID = pros::motor_pid_s();
        // jav do this
        Robot::GetMotor(BotMotorID::CLAW)->vectorPID(clawPID);
    }

    void Execute(std::vector<ComponentAction*>& actions) override {
        if(Components::IsActive(actions, ActionType::CLAW_FOLD_DOWN)) {
            // this needs testing
            Robot::GetMotor(BotMotorID::CLAW)->vectorPositionAbsolute(0, 8);
        }
    }
};
std::vector<BotComponent *> BotComponent::allComponents;
std::vector<ComponentAction *> BotComponent::queue;

BotComponent::BotComponent(std::string name, std::vector<ActionType> validActions) :
        validActions(std::move(validActions)),
        name(std::move(name)) {
    BotComponent::allComponents.push_back(this);
}

bool Components::IsActive(std::vector<ComponentAction *> &actions, ActionType actionType) {
    for (ComponentAction *action : actions) {
        if (action->type == actionType) {
            return true;
        }
    }
    return false;
}

int Components::GetValue(std::vector<ComponentAction *> &actions, ActionType actionType) {
    for (ComponentAction *action : actions) {
        if (action->type == actionType) {
            return action->value;
        }
    }
    return ACTION_TYPE_NOT_ACTIVE;
}

void Components::Execute(ActionType actionType, int value) {
    ComponentAction action = {};
    action.value = value;
    action.type = actionType;
    BotComponent::queue.push_back(&action);
}

void Components::Update() {
    for (BotComponent *component : BotComponent::allComponents) {
        std::vector<ComponentAction *> actionsToSend;
        for (ComponentAction *action : BotComponent::queue) {
            for (ActionType validActionType : component->validActions) {
                if (action->type == validActionType) {
                    actionsToSend.push_back(action);
                }
            }
        }
        component->Execute(actionsToSend);
    }
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
