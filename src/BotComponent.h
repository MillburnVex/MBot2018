#pragma once

#include <vector>
#include "ComponentAction.h"

class BotComponent {
public:
    static std::vector<BotComponent*> allComponents;

    std::vector<ComponentAction> validActions;
    std::vector<ComponentAction> activeActions{};

    std::string name;

    BotComponent(std::string name, std::vector<ComponentAction> validActions);

    bool IsActive(ComponentAction action);

    virtual void Execute(ComponentAction action, int value) {}

    virtual void Update() {}
};

namespace Components {

    void Execute(ComponentAction action, int value);

    void Update();

    void Init();
}
