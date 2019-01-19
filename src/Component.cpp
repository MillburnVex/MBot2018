#include <string>
#include <utility>
#include <algorithm>
#include <functional>
#include "api.h"
#include "Component.h"
#include "Robot.h"
#include "Command.h"
#include "PID.h"

class FlywheelComponent : public BotComponent {
    PID pid = PID(1.5f, 0.0f, 0.7f, 1000, -1000);
public:

    int rpm = 600;
    int voltage = 127;

    FlywheelComponent() : BotComponent("Flywheel component",
                                       {
                                               ActionType::FLYWHEEL_RUN
                                       }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
        if (Components::IsActive(actions, ActionType::FLYWHEEL_RUN)) {
            rpm = Components::GetValue(actions, ActionType::FLYWHEEL_RUN);
        }
        double actualRpm = Robot::GetMotor(MotorID::FLYWHEEL)->GetVelocity();
        int voltageChange = pid.GetValue(actualRpm, rpm);
        voltage += voltageChange;
        Robot::GetMotor(MotorID::FLYWHEEL)->SetVoltage(voltage);
    }
};

class ArmComponent : public BotComponent {
    PID pid = PID(1.5f, 0.0f, 0.5f, 1000, -1000);
public:
    ArmComponent() : BotComponent("Arm component",
                                  {
                                          ActionType::ARM_SET
                                  }) {
        Robot::GetMotor(MotorID::ARM)->GetProsMotor()->tare_position();
    }

    void Execute(std::vector<ComponentAction> &actions) override {
        int voltage = pid.GetValue(
                Robot::GetMotor(MotorID::ARM)->GetPosition(),
                Components::GetValue(actions, ActionType::ARM_SET));
        Robot::GetMotor(MotorID::ARM)->SetVoltage(std::clamp(voltage, -127, 127));
    }
};

class IndexerComponent : public BotComponent {
    int target;
    PID pid = PID(5.0f, 0.0f, 0.5f, 1000, -1000);

public:
    IndexerComponent() : BotComponent("Indexer component",
                                      {
                                              ActionType::INDEXER_RUN
                                      }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
        if (Components::IsActive(actions, ActionType::INDEXER_RUN)) {
            Robot::GetMotor(MotorID::INDEXER)->SetVoltage(Components::GetValue(actions, ActionType::INDEXER_RUN));
            target = Robot::GetMotor(MotorID::INDEXER)->GetPosition();
        } else {
            int voltage = pid.GetValue(
                    Robot::GetMotor(MotorID::INDEXER)->GetPosition(),
                    target);
            Robot::GetMotor(MotorID::INDEXER)->SetVoltage(std::clamp(voltage, -127, 127));
        }
    }
};

class DriveComponent : public BotComponent {

    const double MAX_ACCELERATION = 10;

    const double LINEAR_TOTAL_ERROR_THRESHOLD = 30;

    const double ROTATE_TOTAL_ERROR_THRESHOLD = 10;

    const double STOPPED_VELOCITY_TOTAL_ERROR_THRESHOLD = 5;

    std::array<std::pair<MotorID, double> *, 4> initialPositions{
            new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)
    };
    std::array<std::pair<MotorID, double> *, 4> currentPositions{
            new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)
    };
    std::array<std::pair<MotorID, double> *, 4> lastPositions{
            new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)};
    std::array<std::pair<MotorID, PID> *, 4> pids{
            new std::pair<MotorID, PID>(DRIVE_RIGHT_FRONT, PID(0.8f, 0.0f, 1.0f, 1000, -1000)),
            new std::pair<MotorID, PID>(DRIVE_RIGHT_BACK, PID(0.8f, 0.0f, 1.0f, 1000, -1000)),
            new std::pair<MotorID, PID>(DRIVE_LEFT_FRONT, PID(0.8f, 0.0f, 1.0f, 1000, -1000)),
            new std::pair<MotorID, PID>(DRIVE_LEFT_BACK, PID(0.8f, 0.0f, 1.0f, 1000, -1000))
    };
public:
    DriveComponent() : BotComponent("Drive component",
                                    {
                                            ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE, ActionType::LINEAR_TO,
                                            ActionType::ROTATE_TO
                                    }) {

    }

    void ForEachMotor(std::function<void(MotorID)> func) {
        func(DRIVE_RIGHT_FRONT);
        func(DRIVE_RIGHT_BACK);
        func(DRIVE_LEFT_FRONT);
        func(DRIVE_LEFT_BACK);
    }

    double GetValue(std::array<std::pair<MotorID, double> *, 4> motors, MotorID id) {
        for (auto motorAndValue : motors) {
            if (motorAndValue->first == id) {
                return motorAndValue->second;
            }
        }
        return 0;
    }

    void SetValue(std::array<std::pair<MotorID, double> *, 4> motors, MotorID id, double value) {
        for (auto motorAndValue : motors) {
            if (motorAndValue->first == id) {
                motorAndValue->second = value;
                break;
            }
        }
    }

    PID GetPID(MotorID id) {
        for (auto motorAndValue : pids) {
            if (motorAndValue->first == id) {
                return motorAndValue->second;
            }
        }
        throw "PID does not exist";
    }

    MotorID GetMotor(bool right, bool front) {
        if (right) {
            if (front) {
                return DRIVE_RIGHT_FRONT;
            } else {
                return DRIVE_RIGHT_BACK;
            }
        } else {
            if (front) {
                return DRIVE_LEFT_FRONT;
            } else {
                return DRIVE_LEFT_BACK;
            }
        }
    }

    /**
     * This stores the last positions of the motors and updates the current ones, so that every method that needs them
     * gets the same value
     */
    void UpdatePositions() {
        // previous current ones are the last ones
        ForEachMotor([&](MotorID id) {
            SetValue(lastPositions, id, GetValue(currentPositions, id));
        });
        // new current ones
        ForEachMotor([&](MotorID id) {
            SetValue(currentPositions, id, Robot::GetMotor(id)->GetPosition());
        });
    }

    /**
     * This stores the positions of the motors when not being used in LINEAR/ROTATE_TO, so that when they are called,
     * they can rotate relative to their positions and not absolute from starting position
     */
    void UpdateInitialPositions() {
        ForEachMotor([&](MotorID id) {
            SetValue(initialPositions, id, Robot::GetMotor(id)->GetPosition());
        });
    }

    double MinAbs(double a, double b) {
        if (std::abs(a) < std::abs(b)) {
            return a;
        }
        return b;
    }

    double MinAbs(double a, double b, double c, double d) {
        return MinAbs(MinAbs(a, b), MinAbs(c, d));
    }

    int Sign(double a) {
        if (a < 0) {
            return -1;
        }
        return 1;
    }

    /**
     * This velocity has a units of (motor encoder units--default is degrees) / (Robot::GetUpdateMillis() / 1000) second,
     * that is, the coefficient of its denominator is not 1. If everything else has the same denominator then
     * the calculations will work out well.
     * @param id the ID of the motor to get the velocity of, as long as it is a drive motor
     */
    double GetActualVelocity(MotorID id) {
        return GetValue(currentPositions, id) - GetValue(lastPositions, id);
    }

    double GetGoalVelocity(MotorID id, int goalPosition) {

        double actualVelocity = GetActualVelocity(id);
        double unsmoothedGoalVelocity = GetPID(id).GetValue(GetValue(currentPositions, id), goalPosition);

        double unclampedGoalAcceleration = unsmoothedGoalVelocity - actualVelocity;

        double clampedGoalAcceleration = std::clamp(unclampedGoalAcceleration, -MAX_ACCELERATION, MAX_ACCELERATION);
        double smoothedGoalVelocity = actualVelocity + clampedGoalAcceleration;

        return smoothedGoalVelocity;
    }

    bool NotMoving() {
        return (std::abs(GetActualVelocity(DRIVE_RIGHT_FRONT)) +
                std::abs(GetActualVelocity(DRIVE_RIGHT_BACK)) +
                std::abs(GetActualVelocity(DRIVE_LEFT_FRONT)) +
                std::abs(GetActualVelocity(DRIVE_LEFT_BACK))) / 4 < STOPPED_VELOCITY_TOTAL_ERROR_THRESHOLD;
    }

    void Drive(int right, int left) {
        Robot::GetMotor(MotorID::DRIVE_RIGHT_FRONT)->SetVoltage(std::clamp(right, -127, 127));
        Robot::GetMotor(MotorID::DRIVE_RIGHT_BACK)->SetVoltage(std::clamp(right, -127, 127));
        Robot::GetMotor(MotorID::DRIVE_LEFT_FRONT)->SetVoltage(std::clamp(left, -127, 127));
        Robot::GetMotor(MotorID::DRIVE_LEFT_BACK)->SetVoltage(std::clamp(left, -127, 127));
    }

    bool WithinThreshold(int rightGoalPositionRelative, int leftGoalPositionRelative, double threshold) {
        double rightFrontError = std::abs((GetValue(initialPositions, DRIVE_RIGHT_FRONT) + rightGoalPositionRelative) -
                                          (GetValue(currentPositions, DRIVE_RIGHT_FRONT)));
        double rightBackError = std::abs((GetValue(initialPositions, DRIVE_RIGHT_BACK) + rightGoalPositionRelative) -
                                         (GetValue(currentPositions, DRIVE_RIGHT_BACK)));
        double leftFrontError = std::abs((GetValue(initialPositions, DRIVE_LEFT_FRONT) + leftGoalPositionRelative) -
                                         (GetValue(currentPositions, DRIVE_LEFT_FRONT)));
        double leftBackError = std::abs((GetValue(initialPositions, DRIVE_LEFT_BACK) + leftGoalPositionRelative) -
                                        (GetValue(currentPositions, DRIVE_LEFT_BACK)));
        return (rightFrontError + rightBackError + leftFrontError + leftBackError) / 4 < threshold;
    }

    void LinearTo(int goalPositionRelative) {

        UpdatePositions();

        if (WithinThreshold(goalPositionRelative, goalPositionRelative, LINEAR_TOTAL_ERROR_THRESHOLD)) {
            Drive(0, 0);
            if (NotMoving()) {
                Commands::Release(C_DRIVE_LINEAR_TO);
                UpdateInitialPositions();
            }
            return;
        }

        double rightFrontActualVel = GetActualVelocity(DRIVE_RIGHT_FRONT);
        double rightBackActualVel = GetActualVelocity(DRIVE_RIGHT_BACK);
        double leftFrontActualVel = GetActualVelocity(DRIVE_LEFT_FRONT);
        double leftBackActualVel = GetActualVelocity(DRIVE_LEFT_BACK);

        double rightFrontGoalVel = GetGoalVelocity(DRIVE_RIGHT_FRONT,
                                                   GetValue(initialPositions, DRIVE_RIGHT_FRONT) +
                                                   goalPositionRelative);
        double rightBackGoalVel = GetGoalVelocity(DRIVE_RIGHT_BACK,
                                                  GetValue(initialPositions, DRIVE_RIGHT_BACK) + goalPositionRelative);
        double leftFrontGoalVel = GetGoalVelocity(DRIVE_LEFT_FRONT,
                                                  GetValue(initialPositions, DRIVE_LEFT_FRONT) + goalPositionRelative);
        double leftBackGoalVel = GetGoalVelocity(DRIVE_LEFT_BACK,
                                                 GetValue(initialPositions, DRIVE_LEFT_BACK) + goalPositionRelative);
        // now you have the smoothed velocities for each motor. They aren't dependent on each other
        // the motors should end up going at the same speed. This would work in an ideal world where they go at exactly
        // the values given here, but they don't, so we want them to go at the speed of the lowest motor of the group.

        double lowestGoal = MinAbs(rightFrontGoalVel, rightBackGoalVel, leftFrontGoalVel, leftBackGoalVel);

        double lowestActual = MinAbs(rightFrontActualVel, rightBackActualVel, leftFrontActualVel, leftBackActualVel);

        double finalGoalVel = MinAbs(lowestActual, lowestGoal);

        Drive(std::floor(finalGoalVel), std::floor(finalGoalVel));
    }

    void RotateTo(int goalPositionRelative) {

        UpdatePositions();

        if (WithinThreshold(-goalPositionRelative, goalPositionRelative, ROTATE_TOTAL_ERROR_THRESHOLD)) {
            Commands::Release(C_DRIVE_ROTATE_TO);
            Drive(0, 0);
            UpdateInitialPositions();
            return;
        }

        double rightFrontActualVel = GetActualVelocity(DRIVE_RIGHT_FRONT);
        double rightBackActualVel = GetActualVelocity(DRIVE_RIGHT_BACK);
        double leftFrontActualVel = GetActualVelocity(DRIVE_LEFT_FRONT);
        double leftBackActualVel = GetActualVelocity(DRIVE_LEFT_BACK);

        double rightFrontGoalVel = GetGoalVelocity(DRIVE_RIGHT_FRONT, GetValue(initialPositions, DRIVE_RIGHT_FRONT) -
                                                                      goalPositionRelative);
        double rightBackGoalVel = GetGoalVelocity(DRIVE_RIGHT_BACK,
                                                  GetValue(initialPositions, DRIVE_RIGHT_BACK) - goalPositionRelative);
        double leftFrontGoalVel = GetGoalVelocity(DRIVE_LEFT_FRONT,
                                                  GetValue(initialPositions, DRIVE_LEFT_FRONT) + goalPositionRelative);
        double leftBackGoalVel = GetGoalVelocity(DRIVE_LEFT_BACK,
                                                 GetValue(initialPositions, DRIVE_LEFT_BACK) + goalPositionRelative);
        // now you have the smoothed velocities for each motor. They aren't dependent on each other
        // the motors should end up going at the same speed. This would work in an ideal world where they go at exactly
        // the values given here, but they don't, so we want them to go at the speed of the lowest motor of the group.

        double lowestRightGoal = MinAbs(rightFrontGoalVel, rightBackGoalVel);
        double lowestLeftGoal = MinAbs(leftFrontGoalVel, leftBackGoalVel);

        double lowestRightActual = MinAbs(rightFrontActualVel, rightBackActualVel);
        double lowestLeftActual = MinAbs(leftFrontActualVel, leftBackActualVel);
        // all need to have the same absolute

        double finalRightGoalVel = MinAbs(lowestRightGoal, lowestRightActual);
        double finalLeftGoalVel = MinAbs(lowestLeftGoal, lowestLeftActual);

        int rightSign = Sign(finalRightGoalVel);
        int leftSign = Sign(finalLeftGoalVel);

        double finalGoalVel = MinAbs(finalRightGoalVel, finalLeftGoalVel);

        Drive(std::floor(finalGoalVel) * rightSign, std::floor(finalGoalVel) * leftSign);
    }

    void Execute(std::vector<ComponentAction> &actions) override {
        if (Components::IsActive(actions, ActionType::LINEAR_TO) ||
            Components::IsActive(actions, ActionType::ROTATE_TO)) {
            if (Components::IsActive(actions, ActionType::LINEAR_TO)) {
                int linearTo = Components::GetValue(actions, ActionType::LINEAR_TO);
                LinearTo(linearTo);
            } else {
                int rotateTo = Components::GetValue(actions, ActionType::ROTATE_TO);
                RotateTo(rotateTo);
            }
        } else {

            UpdateInitialPositions();

            int linear = Components::GetValue(actions, ActionType::DRIVE_LINEAR);
            int rotate = Components::GetValue(actions, ActionType::DRIVE_ROTATE);
            Drive(linear, rotate);
        }
    }
};

class ReaperComponent : public BotComponent {
public:
    ReaperComponent() : BotComponent("Ball lift component", {
            ActionType::REAPER_RUN
    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
        Robot::GetMotor(MotorID::BALL_LIFT)->SetVoltage(Components::GetValue(actions, ActionType::REAPER_RUN));
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
