#ifndef ADVANCED_INPUT_H
#define ADVANCED_INPUT_H

using namespace std;
#include <SFML/Graphics.hpp>

class AdvancedInput {
private:
	int numberOfFramesInQueue;
	std::vector<float> pastStickInputAngles;
public:
	AdvancedInput(int number_of_frames_in_queue);
	void AddStickInput(float x, float y);
	bool DidPlayerDoQuarterCircleForward(bool facing_right);
};

#endif