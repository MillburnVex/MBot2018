#include <string>
#include <utility>
#include <algorithm>
#include "api.h"
#include "BotComponent.h"
#include "Robot.h"
#include "Command.h"
#include "PID.h"

class FlywheelComponent : public BotComponent {
    PID pid = PID(0.5f, 0.0f, 0.5f, 1000, -1000);
public:
    FlywheelComponent() : BotComponent("Flywheel component",
                                       {
                                               ActionType::FLYWHEEL_RUN
                                       }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
		int speed = Components::GetValue(actions, ActionType::FLYWHEEL_RUN);
		Robot::GetMotor(BotMotorID::FLYWHEEL)->SetVoltage(-speed);
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
	PID pid = PID(5.0f, 0.0f, 0.5f, 1000, -1000);

public:
	IndexerComponent() : BotComponent("Indexer component",
		{
			ActionType::INDEXER_RUN
		}) {}

	void Execute(std::vector<ComponentAction> &actions) override {
		if (Components::IsActive(actions, ActionType::INDEXER_RUN)) {
			Robot::GetMotor(BotMotorID::INDEXER)->SetVoltage(Components::GetValue(actions, ActionType::INDEXER_RUN));
			target = Robot::GetMotor(BotMotorID::INDEXER)->GetProsMotor()->get_position();

		}
		else {
			int voltage = pid.GetValue(
				Robot::GetMotor(BotMotorID::INDEXER)->GetPosition(),
				target);
			Robot::GetMotor(BotMotorID::INDEXER)->SetVoltage(std::clamp(voltage, -127, 127));
		}
	}
};

class DriveComponent : public BotComponent {
	const int maxChange = 15;

	PID leftFront = PID(0.8f, 0.0f, 1.0f, 1000, -1000);
    PID leftBack = PID(0.8f, 0.0f, 1.0f, 1000, -1000);
	PID rightFront = PID(0.8f, 0.0f, 1.0f, 1000, -1000);
    PID rightBack = PID(0.8f, 0.0f, 1.0f, 1000, -1000);
	double latestLeftFront = 0;
    double latestLeftBack = 0;
	double latestRightFront = 0;
    double latestRightBack = 0;

	int lastLeftBackSpeed = 0;
	int lastLeftFrontSpeed = 0;
	int lastRightBackSpeed = 0;
	int lastRightFrontSpeed = 0;
public:
    DriveComponent() : BotComponent("Drive component",
                                    {
											ActionType::DRIVE_LINEAR, ActionType::DRIVE_ROTATE, ActionType::DRIVE_TO, ActionType::ROTATE_TO
                                    }) {}

    void Execute(std::vector<ComponentAction> &actions) override {
		int leftBackVal = Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->GetPosition();
		int leftFrontVal = Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetPosition();
		int rightBackVal = Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->GetPosition();
		int rightFrontVal = Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetPosition();
		if (Components::IsActive(actions, ActionType::DRIVE_TO) || Components::IsActive(actions, ActionType::ROTATE_TO)) {
			int leftBackSpeed = 0;
            int leftFrontSpeed = 0;
            int rightBackSpeed = 0;
            int rightFrontSpeed = 0;

			if (Components::IsActive(actions, ActionType::DRIVE_TO)) {
				int driveTo = Components::GetValue(actions, ActionType::DRIVE_TO);
				int leftBackError = latestLeftBack + driveTo - leftBackVal;
				int leftFrontError = latestLeftFront + driveTo - leftFrontVal;
				int rightBackError = -latestRightBack + driveTo + rightBackVal;
				int rightFrontError = -latestRightFront + driveTo + rightFrontVal;
				if ((std::abs(leftBackError) + std::abs(leftFrontError) + std::abs(rightBackError) + std::abs(rightFrontError)) / 4.0 < 30.0) {
					// we in the right place
					Commands::Release(C_DRIVE_LINEAR_TO);
					Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(0);
					Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(0);
					Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(0);
					Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(0);
					lastLeftBackSpeed = 0;
					lastLeftFrontSpeed = 0;
					lastRightBackSpeed = 0;
					lastRightFrontSpeed = 0;
					return;
				}
				leftBackSpeed = leftBack.GetValue(
					leftBackVal,
					latestLeftBack + driveTo);
                leftFrontSpeed = leftFront.GetValue(
                        leftFrontVal,
                        latestLeftFront + driveTo);
				rightBackSpeed = -rightBack.GetValue(
					-rightBackVal,
					-latestRightBack + driveTo);
                rightFrontSpeed = -rightFront.GetValue(
                        -rightFrontVal,
                        -latestRightFront + driveTo);

				auto rightDriveSpeed = (Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->GetVelocity() + Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->GetVelocity()) / 2;
				auto leftDriveSpeed  = (Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->GetVelocity()  + Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->GetVelocity())  / 2;

				auto rightVelError = rightDriveSpeed - leftDriveSpeed;
				auto leftVelError = leftDriveSpeed - rightDriveSpeed;

				leftBackSpeed -= leftVelError;
				leftFrontSpeed -= leftVelError;

				rightBackSpeed -= rightVelError;
				rightFrontSpeed -= rightVelError;
			} else {
				int rotateTo = Components::GetValue(actions, ActionType::ROTATE_TO);
				int leftBackError = latestLeftBack + rotateTo - leftBackVal;
				int leftFrontError = latestLeftFront + rotateTo - leftFrontVal;
				int rightBackError = -latestRightBack - rotateTo + rightBackVal;
				int rightFrontError = -latestRightFront - rotateTo + rightFrontVal;
				if ((std::abs(leftBackError) + std::abs(leftFrontError) + std::abs(leftFrontError) + std::abs(rightFrontError)) / 4.0 < 10.0) {
					// we in the right place
					Commands::Release(C_DRIVE_ROTATE_TO);
					Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(0);
					Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(0);
					Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(0);
					Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(0);
					lastLeftBackSpeed = 0;
					lastLeftFrontSpeed = 0;
					lastRightBackSpeed = 0;
					lastRightFrontSpeed = 0;
					return;
				}
                leftBackSpeed = leftBack.GetValue(
                        leftBackVal,
                        latestLeftBack + rotateTo);
                leftFrontSpeed = leftFront.GetValue(
                        leftFrontVal,
                        latestLeftFront + rotateTo);
                rightBackSpeed = -rightBack.GetValue(
                        -rightBackVal,
                        -latestRightBack - rotateTo);
                rightFrontSpeed = -rightFront.GetValue(
                        -rightFrontVal,
                        -latestRightFront - rotateTo);
			}
			// to smooth: Math.ceil(desired speed - previous speedo * 0.1) + previous speed
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_FRONT)->SetVoltage(std::clamp(std::clamp(leftFrontSpeed, lastLeftFrontSpeed - maxChange, lastLeftFrontSpeed + maxChange), -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_LEFT_BACK)->SetVoltage(std::clamp(std::clamp(leftBackSpeed, lastLeftBackSpeed - maxChange, lastLeftBackSpeed + maxChange), -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_FRONT)->SetVoltage(std::clamp(std::clamp(rightFrontSpeed, lastRightFrontSpeed - maxChange, lastRightFrontSpeed + maxChange), -127, 127));
			Robot::GetMotor(BotMotorID::DRIVE_RIGHT_BACK)->SetVoltage(std::clamp(std::clamp(rightBackSpeed, lastRightBackSpeed - maxChange, lastRightBackSpeed + maxChange), -127, 127));

			lastLeftBackSpeed = std::clamp(leftBackSpeed, lastLeftBackSpeed - maxChange, lastLeftBackSpeed + maxChange);
			lastLeftFrontSpeed = std::clamp(leftFrontSpeed, lastLeftFrontSpeed - maxChange, lastLeftFrontSpeed + maxChange);
			lastRightBackSpeed = std::clamp(rightBackSpeed, lastRightBackSpeed - maxChange, lastRightBackSpeed + maxChange);
			lastRightFrontSpeed = std::clamp(rightFrontSpeed, lastRightFrontSpeed - maxChange, lastRightFrontSpeed + maxChange);
			
		} else {
			latestLeftFront = leftFrontVal;
			latestRightFront = rightFrontVal;
            latestLeftBack  = leftBackVal;
            latestRightBack = rightBackVal;

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
