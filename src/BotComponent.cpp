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

    void Execute(std::set<ComponentAction *> &actions) override {
        if (Components::IsActive(actions, ActionType::FLYWHEEL_FULL_SPEED)) {
            Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(127);
        } else if (Components::IsActive(actions, ActionType::FLYWHEEL_STOP)) {
            Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(0);
        } else if (Components::IsActive(actions, ActionType::FLYWHEEL_PASSIVE_SPEED)) {
            Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(80);
        }
    }
};

class DriveComponent : public BotComponent {
public:
    DriveComponent() : BotComponent("Drive component",
                                    {
                                            ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE
                                    }) {}

    void Execute(std::set<ComponentAction *> &actions) override {
        int linear = Components::GetValue(actions, ActionType::DRIVE_LINEAR);
        int rotate = Components::GetValue(actions, ActionType::DRIVE_ROTATE);
        if (linear != ACTION_TYPE_NOT_ACTIVE && rotate != ACTION_TYPE_NOT_ACTIVE) {
            // linear and rotate at the same time! yay
        } else if (linear != ACTION_TYPE_NOT_ACTIVE) {
            // pretty sure just setting voltage is fine, cuz the value and the voltage have a domain/range of [-127, 127]
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(linear);
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(linear);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(linear);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(linear);
        } else if (rotate != ACTION_TYPE_NOT_ACTIVE) {
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(rotate);
            Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(rotate);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(-rotate);
            Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(-rotate);
        }
    }
};

class BallLiftComponent : public BotComponent {
public:
    BallLiftComponent() : BotComponent("Ball lift component", {
            ActionType::BALL_LIFT_DOWN, ActionType::BALL_LIFT_STOP, ActionType::BALL_LIFT_UP
    }) {
        pros::motor_pid_s ballLiftPID = pros::motor_pid_s();
        // jav do this
        Robot::GetMotor(BotMotorID::BALL_LIFT)->SetPID(ballLiftPID);
    }

    void Execute(std::set<ComponentAction *> &actions) override {
        if (Components::IsActive(actions, ActionType::BALL_LIFT_DOWN)) {
            // needs tuning. I figure lower is better for now
            Robot::GetMotor(BotMotorID::BALL_LIFT)->SetVelocity(8);
        } else if (Components::IsActive(actions, ActionType::BALL_LIFT_UP)) {
            Robot::GetMotor(BotMotorID::BALL_LIFT)->SetVelocity(-8);
        }
    }
};

class CapLiftComponent : public BotComponent {
public:
    CapLiftComponent() : BotComponent("Cap lift component", {
            ActionType::CAP_LIFT_DOWN, ActionType::CAP_LIFT_DROP, ActionType::CAP_LIFT_HOLD, ActionType::CAP_LIFT_UP
    }) {}

    void Execute(std::set<ComponentAction *> &actions) override {
        if(Components::IsActive(actions, ActionType::CAP_LIFT_UP)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVelocity(8);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_HOLD)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVelocity(0);
        } else if(Components::IsActive(actions, ActionType::CAP_LIFT_DROP)) {
            Robot::GetMotor(BotMotorID::CAP_LIFT)->SetVoltage(0);
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
        pros::motor_pid_s clawPID = pros::motor_pid_s();
        // jav do this
        Robot::GetMotor(BotMotorID::CLAW)->SetPID(clawPID);
    }

    void Execute(std::set<ComponentAction*>& actions) override {
        if(Components::IsActive(actions, ActionType::CLAW_FOLD_DOWN)) {
            // this needs testing
            Robot::GetMotor(BotMotorID::CLAW)->SetPositionAbsolute(0, 8);
        }
    }
};
std::set<BotComponent *> BotComponent::allComponents;
std::set<ComponentAction *> BotComponent::queue;

BotComponent::BotComponent(std::string name, std::set<ActionType> validActions) :
        validActions(std::move(validActions)),
        name(std::move(name)) {
    BotComponent::allComponents.insert(this);
}

bool Components::IsActive(std::set<ComponentAction *> &actions, ActionType actionType) {
    for (ComponentAction *action : actions) {
        if (action->type == actionType) {
            return true;
        }
    }
    return false;
}

int Components::GetValue(std::set<ComponentAction *> &actions, ActionType actionType) {
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
    BotComponent::queue.insert(&action);
}

void Components::Update() {
    for (BotComponent *component : BotComponent::allComponents) {
        std::set<ComponentAction *> actionsToSend;
        for (ComponentAction *action : BotComponent::queue) {
            for (ActionType validActionType : component->validActions) {
                if (action->type == validActionType) {
                    actionsToSend.insert(action);
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
