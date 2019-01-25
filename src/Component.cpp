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
	PID pid = PID(0.07f, 0.0f, 0.06f, 1000, -1000);
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
		voltage = std::clamp(voltage + voltageChange, -127, 127);
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
        Robot::GetMotor(MotorID::INDEXER)->SetVoltage(Components::GetValue(actions, ActionType::INDEXER_RUN));
    }
};

class DriveComponent : public BotComponent {

    const double MAX_ACCELERATION = 4;

	bool beginStop = false;

    const double LINEAR_TOTAL_ERROR_THRESHOLD = 30;

    const double ROTATE_TOTAL_ERROR_THRESHOLD = 30;

    const double STOPPED_VELOCITY_TOTAL_ERROR_THRESHOLD = 4;
	//1210.800000 1300.800000 1244.800000 1248.800000
	//1213.200000 1298.400000 1240.000000 1245.200000
	//1214.400000 1303.600000 1239.600000 1251.200000
    PID linearRotationCorrection = PID(0.7f, 0.1f, 0.12f, 1000, -1000);
	PID rotationRotationCorrection = PID(0.7f, 0.1f, 0.12f, 1000, -1000);

    std::array<std::pair<MotorID, double> *, 4> initialPositions{
            new std::pair<MotorID, double>(DRIVE_RIGHT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_RIGHT_BACK, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_FRONT, 0),
            new std::pair<MotorID, double>(DRIVE_LEFT_BACK, 0)
    };
	std::array<std::pair<MotorID, double> *, 4> goalVoltages{
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
    std::array<std::pair<MotorID, PID> *, 4> pids{
		 new std::pair<MotorID, PID>(DRIVE_RIGHT_FRONT, PID(0.35f, 0.01f, 0.04f, 1000, -1000)),
		 new std::pair<MotorID, PID>(DRIVE_RIGHT_BACK, PID(0.35f, 0.01f, 0.04f, 1000, -1000)),
		 new std::pair<MotorID, PID>(DRIVE_LEFT_FRONT, PID(0.35f, 0.01f, 0.04f, 1000, -1000)),
		 new std::pair<MotorID, PID>(DRIVE_LEFT_BACK, PID(0.35f, 0.01f, 0.04f, 1000, -1000))
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

    // helper functions for getting and setting stuff from those arrays up top. these are used to cut line count a bit
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

    PID* GetPID(MotorID id) {
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

    void UpdatePositions() {
		SetValue(currentPositions, DRIVE_RIGHT_FRONT, Robot::GetMotor(DRIVE_RIGHT_FRONT)->GetPosition());
		SetValue(currentPositions, DRIVE_RIGHT_BACK, Robot::GetMotor(DRIVE_RIGHT_BACK)->GetPosition());
		SetValue(currentPositions, DRIVE_LEFT_FRONT, Robot::GetMotor(DRIVE_LEFT_FRONT)->GetPosition());
		SetValue(currentPositions, DRIVE_LEFT_BACK, Robot::GetMotor(DRIVE_LEFT_BACK)->GetPosition());
    }

	void UpdateGoalVoltages(double rightVoltage, double leftVoltage) {
		SetValue(goalVoltages, DRIVE_RIGHT_FRONT, rightVoltage);
		SetValue(goalVoltages, DRIVE_RIGHT_BACK, rightVoltage);
		SetValue(goalVoltages, DRIVE_LEFT_FRONT, leftVoltage);
		SetValue(goalVoltages, DRIVE_LEFT_BACK, leftVoltage);
	}

	void ResetPIDS() {
		GetPID(DRIVE_RIGHT_FRONT)->Reset();
		GetPID(DRIVE_RIGHT_BACK)->Reset();
		GetPID(DRIVE_LEFT_FRONT)->Reset();
		GetPID(DRIVE_LEFT_BACK)->Reset();
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

    double GetGoalVoltage(MotorID id) {
		return GetValue(goalVoltages, id);
    }

    double GetRPM(MotorID id) {
        return Robot::GetMotor(id)->GetVelocity();
    }

    double GetGoalVoltage(MotorID id, int relativeGoalPosition) {
		double currentPosition = Robot::GetMotor(id)->GetPosition();

        double pidValue = GetPID(id)->GetValue(currentPosition, relativeGoalPosition + GetValue(initialPositions, id));
        double currentVoltage = GetGoalVoltage(id);
        double unsmoothedGoalVoltage = std::clamp(pidValue, -127.0, 127.0);

        double goalAcceleration = unsmoothedGoalVoltage - currentVoltage;
        double clampedGoalAcceleration = std::clamp(goalAcceleration, -MAX_ACCELERATION, MAX_ACCELERATION);

        double smoothedGoalVoltage = currentVoltage + clampedGoalAcceleration;
		printf("current pos: %f, goal pos: %f, pid value: %f, current volt: %f, unsmoothed goal volt: %f smoothed: %f\n", currentPosition, relativeGoalPosition, pidValue, currentVoltage, unsmoothedGoalVoltage, std::clamp(smoothedGoalVoltage, -127.0, 127.0));
        return std::clamp(smoothedGoalVoltage, -127.0, 127.0);
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
                                          (GetRelativePosition(DRIVE_RIGHT_FRONT) + GetRelativePosition(DRIVE_RIGHT_BACK)) / 2);
        double leftError = std::abs(leftGoalPositionRelative -
                                         (GetRelativePosition(DRIVE_LEFT_FRONT) + GetRelativePosition(DRIVE_LEFT_BACK)) / 2);
        return (rightError + leftError) < threshold;
    }

    void LinearTo(int goalPositionRelative) {

        UpdatePositions();
		if (WithinThreshold(goalPositionRelative, goalPositionRelative, LINEAR_TOTAL_ERROR_THRESHOLD) || beginStop) {
			Drive(0, 0);
			UpdateGoalVoltages(0, 0);
			beginStop = true;
			printf("trying to stop linear\n");
			if (NotMoving()) {
				beginStop = false;
				printf("stopped\n");
				printf("%f %f %f %f\n", GetRelativePosition(DRIVE_RIGHT_FRONT), GetRelativePosition(DRIVE_RIGHT_BACK), GetRelativePosition(DRIVE_LEFT_FRONT), GetRelativePosition(DRIVE_LEFT_BACK));
				Commands::Release(C_DRIVE_LINEAR_TO);
				UpdateInitialPositions();
				ResetPIDS();
			}
			return;
		}
        double rightVoltage = (GetGoalVoltage(DRIVE_RIGHT_FRONT, goalPositionRelative) + GetGoalVoltage(DRIVE_RIGHT_BACK, goalPositionRelative)) / 2;
        // -127
        double leftVoltage = (GetGoalVoltage(DRIVE_LEFT_FRONT, goalPositionRelative) + GetGoalVoltage(DRIVE_LEFT_BACK, goalPositionRelative)) / 2;
        // -127

        double rightRpm = (GetRPM(DRIVE_RIGHT_FRONT) + GetRPM(DRIVE_RIGHT_BACK)) / 2;
        // -200
        double leftRpm = (GetRPM(DRIVE_LEFT_FRONT) + GetRPM(DRIVE_LEFT_BACK)) / 2;
        // -170

        double rpmDifference = rightRpm - leftRpm;
        // -30
        double voltageChange = linearRotationCorrection.GetValue(rpmDifference, 0);
        // positive value

		printf("voltage r: %f, l: %f, rpm r: %f, l: %f, diff: %f, neededVoltageChange: %f\n", rightVoltage, leftVoltage, rightRpm, leftRpm, rpmDifference, voltageChange);

		if(std::abs(rightVoltage + voltageChange / 2) < 127) {
		    if(std::abs(leftVoltage - voltageChange / 2) < 127) {
                rightVoltage += voltageChange / 2;
                leftVoltage -= voltageChange / 2;
		    } else {
                rightVoltage += voltageChange;
		    }
        } else {
            leftVoltage -= voltageChange;
		}
		
		UpdateGoalVoltages(rightVoltage, leftVoltage);

        Drive(rightVoltage, leftVoltage);

    }

    void RotateTo(int goalPositionRelative) {

        UpdatePositions();
        if (WithinThreshold(-goalPositionRelative, goalPositionRelative, ROTATE_TOTAL_ERROR_THRESHOLD) || beginStop) {
            Drive(0, 0);
			UpdateGoalVoltages(0, 0);
			printf("stopping rotate\n");

			beginStop = true;
            if (NotMoving()) {
				beginStop = false;
				printf("stopped\n");
				printf("%f %f %f %f\n", GetRelativePosition(DRIVE_RIGHT_FRONT), GetRelativePosition(DRIVE_RIGHT_BACK), GetRelativePosition(DRIVE_LEFT_FRONT), GetRelativePosition(DRIVE_LEFT_BACK));
                Commands::Release(C_DRIVE_ROTATE_TO);
                UpdateInitialPositions();
				ResetPIDS();
            }
            return;
        }

        double rightVoltage = (GetGoalVoltage(DRIVE_RIGHT_FRONT, -goalPositionRelative) + GetGoalVoltage(DRIVE_RIGHT_BACK, -goalPositionRelative)) / 2;
		double leftVoltage = (GetGoalVoltage(DRIVE_LEFT_FRONT, goalPositionRelative) + GetGoalVoltage(DRIVE_LEFT_BACK, goalPositionRelative)) / 2;

        double rightRpm = (GetRPM(DRIVE_RIGHT_FRONT) + GetRPM(DRIVE_RIGHT_BACK)) / 2;
        double leftRpm = (GetRPM(DRIVE_LEFT_FRONT) + GetRPM(DRIVE_LEFT_BACK)) / 2;

        // right should be opposite
        double rpmDifference = rightRpm + leftRpm;
        // 
		double voltageChange = rotationRotationCorrection.GetValue(rpmDifference, 0);
		printf("voltage r: %f, l: %f, rpm r: %f, l: %f, diff: %f, neededVoltageChange: %f\n", rightVoltage, leftVoltage, rightRpm, leftRpm, rpmDifference, voltageChange);
        // returns positive if right is too fast

		if (std::abs(rightVoltage + voltageChange / 2) < 127) {
			if (std::abs(leftVoltage + voltageChange / 2) < 127) {
				rightVoltage += voltageChange / 2;
				leftVoltage += voltageChange / 2;
			}
			else {
				rightVoltage += voltageChange;
			}
		}
		else {
			leftVoltage += voltageChange;
		}

		UpdateGoalVoltages(rightVoltage, leftVoltage);

        Drive(rightVoltage, leftVoltage);
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
            Drive(linear - rotate, linear + rotate);
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
