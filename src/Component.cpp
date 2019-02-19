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
    PID pid = PID(0.1f, 0.001f, 0.06f, 1000, -1000);
public:

    int rpm = 600;
    int voltage = 127;

    FlywheelComponent() : BotComponent("Flywheel component",
                                       {
                                               ActionType::FLYWHEEL_RUN
                                       }) {}

    void Execute(std::vector <ComponentAction> &actions) override {
        if (Components::IsActive(actions, ActionType::FLYWHEEL_RUN)) {
            rpm = Components::GetValue(actions, ActionType::FLYWHEEL_RUN);
        }
        double actualRpm = Robot::GetMotor(MotorID::FLYWHEEL)->GetVelocity();
        int voltageChange = pid.GetValue(actualRpm, rpm);
        voltage = std::clamp(voltage + voltageChange, 70, 127);
        //printf("actual rpm: %f, goal rpm: %d, voltage change: %d, final voltage: %d\n", actualRpm, rpm, voltageChange, voltage);
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

    void Execute(std::vector <ComponentAction> &actions) override {
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

    void Execute(std::vector <ComponentAction> &actions) override {
        Robot::GetMotor(MotorID::INDEXER)->SetVoltage(Components::GetValue(actions, ActionType::INDEXER_RUN));
    }
};

class DriveComponent : public BotComponent {

    const double LINEAR_TOTAL_ERROR_THRESHOLD = 30;

    const double ROTATE_TOTAL_ERROR_THRESHOLD = 10;

    const double STOPPED_VELOCITY_TOTAL_ERROR_THRESHOLD = 10;

    const double MAX_ROTATION_VOLTAGE = 60;

    const double MAX_LINEAR_ROTATION_CORRECTION_VOLTAGE = 10;

    int goalRotation = 0;

    PID rotationPID = PID(0.55f, 0.0f, 2.5f, 1000, -1000);

    PID rotationCorrection = PID(0.65f, 0.0f, 0.0f, 1000, -1000);

    double velocity;

    std::array<std::pair < MotorID, double> *, 4> initialPositions{
        new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)
    };
    std::array<std::pair < MotorID, double> *, 4> goalVoltages{
        new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)
    };
    std::array<std::pair < MotorID, double> *, 4> lastIntendedPositions{
        new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)
    };
    std::array<std::pair < MotorID, double> *, 4> lastIntendedVelocities{
        new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
                new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)
    };
    std::array<std::pair < MotorID, PID> *, 4> linearPids{
        new std::pair<MotorID, PID>(DRIVE_RIGHT_FRONT, PID(0.27f, 0.02f, 0.17f, 300, -300)),
                new std::pair<MotorID, PID>(DRIVE_RIGHT_BACK, PID(0.27f, 0.02f, 0.17f, 300, -300)),
                new std::pair<MotorID, PID>(DRIVE_LEFT_FRONT, PID(0.27f, 0.02f, 0.17f, 300, -300)),
                new std::pair<MotorID, PID>(DRIVE_LEFT_BACK, PID(0.27f, 0.02f, 0.17f, 300, -300))
    };

public:
    DriveComponent() : BotComponent("Drive component",
                                    {
                                            ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE, ActionType::LINEAR_TO,
                                            ActionType::ROTATION_SET, ActionType::ROTATE_ABSOLUTE
                                    }) {

    }

    double GetValue(std::array<std::pair < MotorID, double> *,

    4> motors,
    MotorID id
    ) {
        for (auto motorAndValue : motors) {
            if (motorAndValue->first == id) {
                return motorAndValue->second;
            }
        }
        return 0;
    }

    void SetValue(std::array<std::pair < MotorID, double> *,

    4> motors,
    MotorID id,
    double value
    ) {
        for (auto motorAndValue : motors) {
            if (motorAndValue->first == id) {
                motorAndValue->second = value;
                break;
            }
        }
    }

    PID *GetPID(std::array<std::pair < MotorID, PID> *,

    4> pids,
    MotorID id
    ) {
        for (auto motorAndValue : pids) {
            if (motorAndValue->first == id) {
                return &motorAndValue->second;
            }
        }
        throw "PID does not exist";
    }

    double GetRelativePosition(MotorID id) {
        return Robot::GetMotor(id)->GetPosition() - GetValue(initialPositions, id);
    }

    void UpdateGoalVoltages(double rightVoltage, double leftVoltage) {
        SetValue(goalVoltages, DRIVE_RIGHT_FRONT, rightVoltage);
        SetValue(goalVoltages, DRIVE_RIGHT_BACK, rightVoltage);
        SetValue(goalVoltages, DRIVE_LEFT_FRONT, leftVoltage);
        SetValue(goalVoltages, DRIVE_LEFT_BACK, leftVoltage);
    }

    void ResetPIDS() {
        GetPID(linearPids, DRIVE_RIGHT_FRONT)->Reset();
        GetPID(linearPids, DRIVE_RIGHT_BACK)->Reset();
        GetPID(linearPids, DRIVE_LEFT_FRONT)->Reset();
        GetPID(linearPids, DRIVE_LEFT_BACK)->Reset();
        rotationPID.Reset();
        rotationCorrection.Reset();
    }

    /**
     * This stores the positions of the motors when not being used in LINEAR/ROTATE_TO, so that when they are called,
     * they can rotate relative to their positions and not absolute from starting position
     */
    void UpdateInitialPositions() {
        SetValue(initialPositions, DRIVE_RIGHT_FRONT, Robot::GetMotor(DRIVE_RIGHT_FRONT)->GetPosition());
        SetValue(initialPositions, DRIVE_RIGHT_BACK, Robot::GetMotor(DRIVE_RIGHT_BACK)->GetPosition());
        SetValue(initialPositions, DRIVE_LEFT_FRONT, Robot::GetMotor(DRIVE_LEFT_FRONT)->GetPosition());
        SetValue(initialPositions, DRIVE_LEFT_BACK, Robot::GetMotor(DRIVE_LEFT_BACK)->GetPosition());
    }

    void UpdateInitialPositions(double rightPosition, double leftPosition) {
        SetValue(initialPositions, DRIVE_RIGHT_FRONT, rightPosition);
        SetValue(initialPositions, DRIVE_RIGHT_BACK, rightPosition);
        SetValue(initialPositions, DRIVE_LEFT_FRONT, leftPosition);
        SetValue(initialPositions, DRIVE_LEFT_BACK, leftPosition);
    }

    double GetGoalVoltage(MotorID id) {
        return GetValue(goalVoltages, id);
    }

    double GetRPM(MotorID id) {
        return Robot::GetMotor(id)->GetVelocity();
    }

    bool NotMoving() {
        return (std::abs(GetRPM(DRIVE_RIGHT_FRONT)) +
                std::abs(GetRPM(DRIVE_RIGHT_BACK)) +
                std::abs(GetRPM(DRIVE_LEFT_FRONT)) +
                std::abs(GetRPM(DRIVE_LEFT_BACK))) / 4 < STOPPED_VELOCITY_TOTAL_ERROR_THRESHOLD;
    }

    void Drive(int right, int left) {
        Robot::GetMotor(MotorID::DRIVE_RIGHT_FRONT)->SetVoltage(std::clamp(right, -127, 127));
        Robot::GetMotor(MotorID::DRIVE_RIGHT_BACK)->SetVoltage(std::clamp(right, -127, 127));
        Robot::GetMotor(MotorID::DRIVE_LEFT_FRONT)->SetVoltage(std::clamp(left, -127, 127));
        Robot::GetMotor(MotorID::DRIVE_LEFT_BACK)->SetVoltage(std::clamp(left, -127, 127));
    }


    bool WithinThreshold(int rightGoalPositionRelative, int leftGoalPositionRelative, double threshold) {
        double rightError = std::abs(rightGoalPositionRelative -
                                     (GetRelativePosition(DRIVE_RIGHT_FRONT) + GetRelativePosition(DRIVE_RIGHT_BACK)) /
                                     2);
        double leftError = std::abs(leftGoalPositionRelative -
                                    (GetRelativePosition(DRIVE_LEFT_FRONT) + GetRelativePosition(DRIVE_LEFT_BACK)) / 2);
        return (rightError + leftError) < threshold;
    }

    int Sign(double d) {
        if (d < 0) {
            return -1;
        }
        return 1;
    }

    double GetGoalVoltage(MotorID id, int relativeGoalPosition, std::array<std::pair < MotorID, PID> *,

    4> pids) {
        double currentPosition = Robot::GetMotor(id)->GetPosition();
        double pidValue = GetPID(pids, id)->GetValue(currentPosition,
                                                     relativeGoalPosition + GetValue(initialPositions, id));
        double currentVoltage = GetGoalVoltage(id);
        double unsmoothedGoalVoltage = std::clamp(pidValue, -127.0, 127.0);

        //printf("current pos: %f, goal pos: %f, pid value: %f, current volt: %f, unsmoothed goal volt: %f\n", currentPosition, relativeGoalPosition, pidValue, currentVoltage, unsmoothedGoalVoltage);
        return unsmoothedGoalVoltage;
    }

    void LinearTo(int goalPositionRelative) {

        if (WithinThreshold(goalPositionRelative, goalPositionRelative, LINEAR_TOTAL_ERROR_THRESHOLD) && NotMoving()) {
            Commands::Release(C_DRIVE_LINEAR_TO);
            printf("linear done           -----------------------\n");
            UpdateGoalVoltages(0, 0);
            Drive(0, 0);
            UpdateInitialPositions();
            ResetPIDS();
            return;
        }
        printf("right error %f\n", std::abs(goalPositionRelative -
                                            (GetRelativePosition(DRIVE_RIGHT_FRONT) +
                                             GetRelativePosition(DRIVE_RIGHT_BACK)) / 2));
        printf("left error %f\n", std::abs(goalPositionRelative -
                                           (GetRelativePosition(DRIVE_LEFT_FRONT) +
                                            GetRelativePosition(DRIVE_LEFT_FRONT)) / 2));
        printf("not moving %f\n", (std::abs(GetRPM(DRIVE_RIGHT_FRONT)) +
                                   std::abs(GetRPM(DRIVE_RIGHT_BACK)) +
                                   std::abs(GetRPM(DRIVE_LEFT_FRONT)) +
                                   std::abs(GetRPM(DRIVE_LEFT_BACK))) / 4);
        double rightVoltage = (GetGoalVoltage(DRIVE_RIGHT_FRONT, goalPositionRelative, linearPids) +
                               GetGoalVoltage(DRIVE_RIGHT_BACK, goalPositionRelative, linearPids)) / 2;
        //printf("   right voltage: %f\n", rightVoltage);
        double leftVoltage = (GetGoalVoltage(DRIVE_LEFT_FRONT, goalPositionRelative, linearPids) +
                              GetGoalVoltage(DRIVE_LEFT_BACK, goalPositionRelative, linearPids)) / 2;
        //printf("   left voltage: %f\n", leftVoltage);
        double actual = Robot::GetRotation();
        double error = rotationCorrection.GetValue(actual, goalRotation);
        error = std::clamp(error, -MAX_LINEAR_ROTATION_CORRECTION_VOLTAGE, MAX_LINEAR_ROTATION_CORRECTION_VOLTAGE);
        printf("target %d actual %f pid value %f\n", goalRotation, actual, error);
        //leftVoltage += error;
        //rightVoltage -= error;

        UpdateGoalVoltages(rightVoltage, leftVoltage);

        Drive(rightVoltage, leftVoltage);
    }

    void RotateTo(int goalPositionRelative, bool absolute) {
        int target = goalPositionRelative;
        if (!absolute) target += goalRotation;
        if (std::abs(target - Robot::GetRotation()) < ROTATE_TOTAL_ERROR_THRESHOLD && NotMoving()) {
            Drive(0, 0);
            UpdateGoalVoltages(0, 0);
            Commands::Release(C_DRIVE_ROTATE_TO_ABSOLUTE);
            UpdateInitialPositions();
            ResetPIDS();
            //printf("rotation done              ----------------------\n");
            goalRotation = target;
            return;
        }

        //printf("current value: %d\n", Robot::GetSensor(SensorID::GYRO)->GetValue());
        //printf("goal: %d\n", goalPositionRelative);
        //printf("target: %d\n", target);
        //printf("initial: %d\n", initialRotation);

        double leftVoltage = rotationPID.GetValue(Robot::GetRotation(), target);
        leftVoltage = std::clamp(leftVoltage, -MAX_ROTATION_VOLTAGE, MAX_ROTATION_VOLTAGE);
        double rightVoltage = -leftVoltage;

        Drive(rightVoltage, leftVoltage);
    }

    void Execute(std::vector <ComponentAction> &actions) override {
        if (Components::IsActive(actions, ActionType::LINEAR_TO) ||
            Components::IsActive(actions, ActionType::ROTATION_SET) ||
            Components::IsActive(actions, ActionType::ROTATE_ABSOLUTE)) {
            if (Components::IsActive(actions, ActionType::LINEAR_TO)) {
                int linearTo = Components::GetValue(actions, ActionType::LINEAR_TO);
                LinearTo(linearTo);
            } else {
                if (Components::IsActive(actions, ActionType::ROTATION_SET)) {
                    goalRotation = Components::GetValue(actions, ActionType::ROTATION_SET);
                    Commands::Release(C_DRIVE_SET_ROTATION);
                } else {
                    int rotateTo = Components::GetValue(actions, ActionType::ROTATE_ABSOLUTE);
                    RotateTo(rotateTo, true);
                }
            }
        } else {
            UpdateInitialPositions();
            int linear = Components::GetValue(actions, ActionType::DRIVE_LINEAR);
            int rotate = Components::GetValue(actions, ActionType::DRIVE_ROTATE);
            Drive(linear - rotate, linear + rotate);
        }
    }
};

DriveComponent *drive = NULL;

class ReaperComponent : public BotComponent {
public:
    ReaperComponent() : BotComponent("Ball lift component", {
            ActionType::REAPER_RUN
    }) {}

    void Execute(std::vector <ComponentAction> &actions) override {
        Robot::GetMotor(MotorID::BALL_LIFT)->SetVoltage(Components::GetValue(actions, ActionType::REAPER_RUN));
    }
};

std::vector<BotComponent *> BotComponent::allComponents;
std::vector <ComponentAction> BotComponent::queue;

BotComponent::BotComponent(std::string name, std::vector <ActionType> validActions) :
        validActions(std::move(validActions)),
        name(std::move(name)) {
    BotComponent::allComponents.push_back(this);
}

bool Components::IsActive(std::vector <ComponentAction> &actions, ActionType actionType) {
    for (ComponentAction action : actions) {
        if (action.type == actionType) {
            return true;
        }
    }
    return false;
}

int Components::GetValue(std::vector <ComponentAction> &actions, ActionType actionType) {
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
    std::vector <ComponentAction> actionsToSend;
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

void Components::ResetInitialPositions() {
    drive->UpdateInitialPositions(0, 0);
}

void Components::Init() {
    drive = new DriveComponent();
    new FlywheelComponent();
    new ReaperComponent();
    new IndexerComponent();
    new ArmComponent();
}

void Components::Execute(ActionType actionType) {
    Execute(actionType, 0);
}
