#pragma once

#include <vector>
#include <string>
#include "ComponentAction.h"

class BotComponent {
public:
    static std::vector<BotComponent *> allComponents;

    std::vector<ComponentAction> validActions;
    std::vector<ComponentAction> activeActions{};

    std::string name;

    BotComponent(std::string name, std::vector<ComponentAction> validActions);

    virtual void Execute(ComponentAction action, int value) {
    }
};

namespace Components {

    void Execute(ComponentAction action);

    void Execute(ComponentAction action, int value);

    void Init();
}
