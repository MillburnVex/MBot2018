#include <string>
#include <utility>
#include "api.h"
#include "BotComponent.h"

std::vector<BotComponent *> BotComponent::allComponents;

BotComponent::BotComponent(std::string name, std::vector<ComponentAction> validActions) :
        validActions(std::move(validActions)),
        name(std::move(name)) {
    BotComponent::allComponents.push_back(this);
}

class FlywheelComponent : public BotComponent {
public:
    FlywheelComponent() : BotComponent("Flywheel component",
                                       {ComponentAction::FLYWHEEL_FULL_SPEED, ComponentAction::FLYWHEEL_STOP}) {}

    void Execute(ComponentAction action, int value) override {
        if (action == ComponentAction::FLYWHEEL_FULL_SPEED) {

        } else if (action == ComponentAction::FLYWHEEL_STOP) {

        }
    }

    void Update() override {
        if (IsActive(ComponentAction::FLYWHEEL_FULL_SPEED)) {
            // motor to full power
        } else if (IsActive(ComponentAction::FLYWHEEL_STOP)) {
            // motor to 0 power
        }
    }
};

class DriveComponent : public BotComponent {
public:
    DriveComponent() : BotComponent("Drive component",
                                    {ComponentAction::DRIVE_LINEAR, ComponentAction::DRIVE_ROTATE}) {}

    void Execute(ComponentAction action, int value) override {
        if (action == ComponentAction::DRIVE_LINEAR) {

        }
    }

    void Update() override {

    }
};

void Components::Execute(ComponentAction action, int value) {
    for (auto component : BotComponent::allComponents) {
        for (auto validAction : component->validActions) {
            if (validAction == action) {
                component->activeActions.push_back(action);
                component->Execute(action, value);
            }
        }
    }
}

void Components::Update() {
    for (auto component : BotComponent::allComponents) {
        component->Update();
    }
}

void Components::Init() {
    new DriveComponent();
    new FlywheelComponent();
}

bool BotComponent::IsActive(ComponentAction action) {
    for (auto a : activeActions) {
        if (a == action) {
            return true;
        }
    }
    return false;
}