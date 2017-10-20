#ifndef WEAPON_H
#define WEAPON_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "PlatformerLibrary\Box2DRigidBody.h"

class Weapon : public Box2DRigidBody {
private:
	float stuck_angle;
	int player_index;
	bool stuck;
	bool recalling;
	float gravityScale;
	b2BodyDef weaponBodyDef;
	b2Body* weaponBody;
	b2PolygonShape weaponShape;
	b2FixtureDef weaponFixtureDef;
	b2Fixture* weaponFixture;
	b2Fixture* stuck_fixture;
	b2Vec2 stuck_position_offset;
	b2Body* ownersBody;
public:
	Weapon(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity, int player_idx, b2Body* owners_body);
	void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
	void Throw(b2Vec2 vector, b2Vec2 starting_position);
	void Stick(b2Fixture* stuck_fix, float angle);
	void Recall();
	void TeleportedTo();

	b2Body* GetBody() {
		return weaponBody;
	}
};

#endif