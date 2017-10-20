#pragma once

using namespace std;
#include <SFML/Graphics.hpp>
#include "../ControllableCharacter.h"

class InputHandler {
private:
	//static int next_player_index;
	int number_of_frames_to_eat_inputs;
	int a_button = 0;
	int b_button = 1;
	int x_button = 2;
	int y_button = 3;
	int right_bumper_button = 4;
	int left_bumper_button = 5;
	int select_button = 6;
	int start_button = 7;
	bool a_button_previous = false;
	bool b_button_previous = false;
	bool x_button_previous = false;
	bool y_button_previous = false;
	bool right_bumper_button_previous = false;
	bool left_bumper_button_previous = false;
	bool select_button_previous = false;
	bool start_button_previous = false;
	float deadzone = 20.0f;
	ControllableCharacter* player_character;
public:
	InputHandler(ControllableCharacter* pc);
	void Update();
	void EatInputsForNumberOfFrames(int number_of_frames);
	//static int GetAvailablePlayerIndex();
};