#pragma once

typedef enum {
	FLYWHEEL_RUN,
	DRIVE_LINEAR, DRIVE_ROTATE, LINEAR_TO, ROTATION_SET, ROTATE_ABSOLUTE,

	REAPER_RUN,

	INDEXER_RUN,
	ARM_SET

} ActionType;

struct ComponentAction {
    ActionType type;
    int value;
};