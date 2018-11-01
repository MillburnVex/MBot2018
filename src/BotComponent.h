#pragma once

#include <vector>
#include <string>
#include <set>
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
    static std::set<BotComponent *> allComponents;
    static std::set<ComponentAction *> queue;

    std::set<ActionType> validActions;

    std::string name;

    BotComponent(std::string name, std::set<ActionType> validActions);

    virtual void Execute(std::set<ComponentAction*>& actions) {
    }
};

namespace Components {

    bool IsActive(std::set<ComponentAction*>& actions, ActionType actionType);

    int GetValue(std::set<ComponentAction*>& actions, ActionType actionType);

    void Execute(ActionType actionType);

    void Execute(ActionType actionType, int value);

    void Update();

    void Init();
}
