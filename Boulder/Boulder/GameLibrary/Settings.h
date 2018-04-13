#ifndef SETTINGS_H
#define SETTINGS_H

using namespace std;
#include <SFML/Graphics.hpp>

class Settings {
public:
	float music_volume;
	float effects_volume;
	bool fullscreen;
	bool using_arrows_for_movement;
	Settings();
};

#endif