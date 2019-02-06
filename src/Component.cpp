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

	bool beginStop = false;

    const double MAX_ACCELERATION = 4;

    const double LINEAR_TOTAL_ERROR_THRESHOLD = 30;

    const double ROTATE_TOTAL_ERROR_THRESHOLD = 10;

    const double STOPPED_VELOCITY_TOTAL_ERROR_THRESHOLD = 4;

	double initialRotation = 0;

    PID linearRotationCorrection = PID(0.0f, 0.0f, 0.0f, 1000, -1000);

	PID linearPID = PID(2f, 0.0f, 0.5f, 1000, -1000);
	PID rotationPID = PID(0.3f, 0.0f, 0.5f, 1000, -1000);

	float velocity;

	float position;
	
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

	void ResetPIDS() {
		linearPID.Reset();
		rotationPID.Reset();
	}

	/**
	 * This stores the positions of the motors when not being used in LINEAR/ROTATE_TO, so that when they are called,
	 * they can rotate relative to their positions and not absolute from starting position
	 */
	void UpdateInitialPositions() {
		initialRotation = Robot::GetSensor(SensorID::GYRO)->GetValue();
		position = 0;
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

    void LinearTo(int goalPositionRelative) {
		if (std::abs(position - goalPositionRelative) < LINEAR_TOTAL_ERROR_THRESHOLD || beginStop) {
			Drive(0, 0);
			beginStop = false;
			Commands::Release(C_DRIVE_LINEAR_TO);
			UpdateInitialPositions();
			ResetPIDS();
			
			return;
		}

		position += velocity * (Robot::GetUpdateMillis() / 1000);

		double rightVoltage = linearPID.GetValue(position, goalPositionRelative);
		rightVoltage = std::clamp(rightVoltage, -127, 127);
		double leftVoltage = -rightVoltage;

        Drive(rightVoltage, leftVoltage);

    }

    void RotateTo(int goalPositionRelative) {
        if (std::abs((goalPositionRelative - initialRotation) - Robot::GetSensor(SensorID::GYRO)->GetValue()) < ROTATE_TOTAL_ERROR_THRESHOLD || beginStop) {
			Drive(0, 0);
			beginStop = false;
            Commands::Release(C_DRIVE_ROTATE_TO);
            UpdateInitialPositions();
			ResetPIDS(); 

            return;
        }

		double leftVoltage = rotationPID.GetValue(Robot::GetSensor(SensorID::GYRO)->GetValue(), goalPositionRelative - initialRotation);
		leftVoltage = std::clamp(leftVoltage, -90.0, 90.0);
		double rightVoltage = -leftVoltage;

        Drive(rightVoltage, leftVoltage);
    }

    void Execute(std::vector<ComponentAction> &actions) override {
		velocity += static_cast<Accelerometer*>(Robot::GetSensor(SensorID::ACCELEROMETER))->GetX() * (Robot::GetUpdateMillis() / 1000.0);
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
