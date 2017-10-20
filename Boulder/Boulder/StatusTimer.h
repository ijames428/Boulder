#ifndef STATUS_TIMER_H
#define STATUS_TIMER_H

using namespace std;
#include <SFML/Graphics.hpp>

class StatusTimer {
private:
	sf::Int64 starting_frame;
	sf::Int64 duration_in_frames;
public:
	StatusTimer(sf::Int32 dur_in_frames);
	bool IsActive();
	void Start();
};

#endif