#include "Command.h"
#include "BotComponent.h"

std::vector<Command*> Command::allCommands;
std::vector<int> Command::controlsLastActive;

class DriveCommands : public Command {
public:
DriveCommands() : Command(pros::E_CONTROLLER_MASTER,
	{
		pros::E_CONTROLLER_ANALOG_RIGHT_Y, pros::E_CONTROLLER_ANALOG_RIGHT_X
	}) {}

void Execute(std::vector<ControlPress*> &values) override {
	int linear = Commands::GetValue(values, pros::E_CONTROLLER_ANALOG_RIGHT_Y);
	int rotation = Commands::GetValue(values, pros::E_CONTROLLER_ANALOG_RIGHT_X);
	Components::Execute(ComponentAction::DRIVE_LINEAR, linear);
	Components::Execute(ComponentAction::DRIVE_ROTATE, rotation);
}
};

class BallLiftCommands : public Command {
public:
BallLiftCommands() : Command(pros::E_CONTROLLER_MASTER,
	{
		pros::E_CONTROLLER_DIGITAL_R1, pros::E_CONTROLLER_DIGITAL_R2
	}) {}

void Execute(std::vector<ControlPress*> &values) override {
	int up = (Commands::GetValue(values, pros::E_CONTROLLER_DIGITAL_R1) == PressType::PRESSED);
	int down = (Commands::GetValue(values, pros::E_CONTROLLER_DIGITAL_R2) == PressType::PRESSED);
	if(Commands::GetValue(values, pros::E_CONTROLLER_DIGITAL_R1) == PressType::PRESSED) {

	}
}
};

void Commands::Init() {
	new DriveCommands();
	new BallLiftCommands();
}

Command::Command(pros::controller_id_e_t type, std::vector<int> controls) : type(type), controls(std::move(controls)) {
	Command::allCommands.push_back(this);
}

int Commands::GetValue(std::vector<ControlPress*> &vec, int control) {
	for (ControlPress* t : vec) {
		if (t->control == control)
			return t->value;
	}
	return CONTROL_NOT_ACTIVE;
}

PressType Commands::GetPressType(std::vector<ControlPress*> &vec, int control) {
	for (ControlPress* t : vec) {
		if (t->control == control)
			return t->pressType;
	}
	throw "No control with the matching id in the std::vector<ControlPress*>";
}

bool Commands::Contains(std::vector<int> &vec, int i) {
	for (int b : vec) {
		if (b == i)
			return true;
	}
	return false;
}

bool Commands::Contains(std::vector<ControlPress*> presses, int control) {
	for (ControlPress* press : presses) {
		if (press->control == control) {
			return true;
		}
	}
	return false;
}

bool Commands::Contains(std::vector<int> &vec, std::vector<int> &i) {
	bool ret = true;
	for (int a : i) {
		if (!Contains(vec, a)) {
			ret = false;
		}
	}
	return ret;
}

void Commands::Update() {
	pros::Controller master(pros::E_CONTROLLER_MASTER);
	std::vector<ControlPress*> newControls{};

	std::vector<ControlPress*> masterControls{};

	// loop through all the values in the controller_digital_e_t enum
	for (int i = pros::E_CONTROLLER_DIGITAL_L1; i <= pros::E_CONTROLLER_DIGITAL_A; i++) {
		auto button = static_cast<pros::controller_digital_e_t>(i);
		if (master.get_digital(button)) {
			ControlPress press = {};
			press.control = button;
			press.value = 1;
			// press type assigned later
			newControls.push_back(&press);
		}
	}
	// loop through all the values in the controller_analog_e_t enum
	for (int i = pros::E_CONTROLLER_ANALOG_LEFT_X; i <= pros::E_CONTROLLER_ANALOG_RIGHT_Y; i++) {
		auto control = static_cast<pros::controller_analog_e_t>(i);
		int value = master.get_analog(control);
		if (value < -ANALOG_CONTROL_ACTIVE_THRESHOLD || value > ANALOG_CONTROL_ACTIVE_THRESHOLD) {
			ControlPress press = {};
			press.control = control;
			press.value = value;
			// press type assigned later
			newControls.push_back(&press);
		}
	}
	// check for new presses and repeated presses
	for (ControlPress* i : newControls) {
		
		ControlPress press = {};
		// assign press type
		
		if (Contains(Command::controlsLastActive, i->control)) {
			//printf("control %d has been repeated\n", i->control);
			press.pressType = PressType::REPEATED;
		} else {
			//printf("control %d PRESSED\n", i->control);
			press.pressType = PressType::PRESSED;
		}
		
		press.control = i->control;
		
		press.value = i->value;
		// this fucker
		masterControls.push_back(&press);
		
	}
	printf("new controls:\n");
	for (ControlPress* press : newControls) {
		printf("    control %d\n", press->control);
	}
	// check for releases
	for (int i : Command::controlsLastActive) {
		if (!Contains(newControls, i)) {
			ControlPress press = {};
			press.pressType = PressType::RELEASED;
			press.control = i;			press.value = 0;
			masterControls.push_back(&press);
		}
	}
	for (Command* command : Command::allCommands) {
		std::vector<ControlPress*> controlsToSend;
		// determine which commands are active and then send the controls to their execute function
		for (ControlPress* control : masterControls) {
			if (Contains(command->controls, control->control)) {
				// the command is "active" and will be executed
				controlsToSend.push_back(control);
			}
		}
		if (!controlsToSend.empty()) {
			command->Execute(controlsToSend);
		}
	}
	int i = 0;
	for(ControlPress* press : masterControls) {
		i++;
		pros::lcd::print(i, std::to_string(press->control).c_str());
	}
	Command::controlsLastActive.clear();
	for (ControlPress* press : newControls) {
		//printf("press %d in the new contrls\n", press->control);
		Command::controlsLastActive.push_back(press->control);
	}
}
