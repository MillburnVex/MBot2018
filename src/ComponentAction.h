#pragma once

typedef enum {
	FLYWHEEL_RUN,
	DRIVE_LINEAR, DRIVE_ROTATE, DRIVE_TO, ROTATE_TO,

	REAPER_RUN,

	INDEXER_RUN,
	ARM_SET

} ActionType;

struct ComponentAction {
    ActionType type;
    int value;
};