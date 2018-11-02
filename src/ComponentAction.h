#pragma once

typedef enum {
    FLYWHEEL_RUN,
    DRIVE_LINEAR, DRIVE_ROTATE,

    BALL_LIFT_UP, BALL_LIFT_DOWN, BALL_LIFT_STOP,

    CAP_LIFT_UP, CAP_LIFT_DOWN, CAP_LIFT_HOLD, CAP_LIFT_DROP,

    CLAW_FOLD_UP, CLAW_FOLD_DOWN
} ActionType;

struct ComponentAction {
    ActionType type;
    int value;
};