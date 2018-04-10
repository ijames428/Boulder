#ifndef HURT_BOX_H
#define HURT_BOX_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "StatusTimer.h"
#include "GameLibrary\Json\json.h"

class HurtBox {
private:
	b2Fixture* myFixtureLeft;
	b2Fixture* myFixtureRight;
	b2FixtureDef myFixtureDefLeft;
	b2FixtureDef myFixtureDefRight;
	b2PolygonShape polygonShapeLeft;
	b2PolygonShape polygonShapeRight;
	bool facing_right;
public:
	HurtBox(b2Body* body, int player_index, Json::Value jsonData);
	void Update(bool new_facing_right);
	void Activate();
	void Deactivate();
	bool IsFacingRight();
	b2Shape* GetShape();
};

#endif