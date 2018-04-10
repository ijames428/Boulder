#ifndef BOULDER_PROJECTILE_H
#define BOULDER_PROJECTILE_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "GameLibrary\SpriteAnimation.h"
#include "GameLibrary\Json\json.h"
#include "StatusTimer.h"

class BoulderProjectile {
private:
	int damage;
	int projectile_spawn_frame;
	sf::Vector2f projectile_spawn_relative_position;
	sf::Vector2f travel_vector;
	sf::Vector2f knock_back;

	bool is_facing_right;

	sf::RenderWindow *render_window;

	int state;
	const int STATE_INACTIVE = 0;
	const int STATE_TRAVELLING = 1;
	const int STATE_DYING = 2;

	SpriteAnimation* travel_animation;
	SpriteAnimation* on_hit_animation;

	b2BodyDef bodyDef;
	b2Body* body;
	b2PolygonShape shape;
	b2FixtureDef fixture_def;
	b2Fixture* fixture;

	StatusTimer* dying_status_timer;
public:
	BoulderProjectile(sf::RenderWindow* window, b2Body* throwers_body, Json::Value projectile_json_data, SpriteAnimation* travel_anim, SpriteAnimation* on_hit_anim);
	void Update();// sf::Int64 curr_frame, sf::Int64 delta_time);
	void Draw(sf::Vector2f camera_position);
	bool IsActive();
	void Activate(float pos_x, float pos_y, bool facing_right);
	void Hit(); 
	int GetDamage();
	sf::Vector2f GetKnockBack();
	int GetHitStunFrames();
};

#endif