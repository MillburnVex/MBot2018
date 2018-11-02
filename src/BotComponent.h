#pragma once

#include <vector>
#include <string>
#include <vector>
#include "ComponentAction.h"

#define ACTION_TYPE_NOT_ACTIVE 100000

/**
 * A component of the bot, e.g. the ball lift or the flywheel. These should correspond with actual physical components,
 * unlike the commands, which need not.
 * They should ideally act 'dumb,' that is, they shouldn't check for boundaries/sensor values/whatever else and should
 * blindly execute a single function defined by the passed in ComponentAction
 */
class BotComponent {
public:
    static std::vector<BotComponent *> allComponents;
    static std::vector<ComponentAction> queue;

    std::vector<ActionType> validActions;

    std::string name;

    BotComponent(std::string name, std::vector<ActionType> validActions);

    virtual void Execute(std::vector<ComponentAction>& actions) {
    }
};

namespace Components {

    bool IsActive(std::vector<ComponentAction>& actions, ActionType actionType);

    int GetValue(std::vector<ComponentAction>& actions, ActionType actionType);

    void Execute(ActionType actionType);

    void Execute(ActionType actionType, int value);

    void Update();

    void Init();
}
