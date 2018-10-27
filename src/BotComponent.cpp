#include <vector>
#include "ComponentAction.cpp"

class BotComponent {
    std::vector<ComponentAction> validActions;

    virtual void Execute(ComponentAction action, int value) {
    }

    virtual void Update() {
    }
};