#ifndef BOSS_ONE_H
#define BOSS_ONE_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "GameLibrary\Json\json.h"
#include "BoulderCreature.h"

class BossOne : public BoulderCreature {
private:
	float distance_from_target_last_frame;
	bool returning_to_ground;
	bool hovering_altitude_reached;
	bool hovering;
public:
	BossOne(string unit_name, string unit_type, string bestiary_name, bool is_npc, Json::Value jsonBestiariesData, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	virtual void UpdateBehavior();
	virtual void Deaggro();
	virtual void Land();
};

#endif