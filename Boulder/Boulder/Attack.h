#ifndef ATTACK_H
#define ATTACK_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "StatusTimer.h"
#include "GameLibrary\Json\json.h"

class HitBox {
private:
	b2Fixture* myFixtureLeft;
	b2Fixture* myFixtureRight;
	b2FixtureDef myFixtureDefLeft;
	b2FixtureDef myFixtureDefRight;
	b2PolygonShape polygonShapeLeft;
	b2PolygonShape polygonShapeRight;
	bool facing_right;
public:
	HitBox(b2Body* body, int player_index, int frame_in_attack, sf::Vector2f dimensions, sf::Vector2f relative_position);
	void Update(bool new_facing_right);
	void Activate();
	void Deactivate();
	bool IsFacingRight();
};

class AttackFrame {
private:
	int damage;
	bool active;
	HitBox* hitbox;
	bool has_hit_box;
	b2Vec2 velocity_facing_right;
	sf::Vector2f knockback_facing_right;
	sf::Vector2f knockback_facing_left;
	int number_of_hit_stun_frames;
	bool frame_for_clearing_enemies_hit;
public:
	AttackFrame(b2Body* body, int player_index, int frame_in_attack, int dmg, sf::Int64 stun_frames, sf::Vector2f kfr, sf::Vector2f hit_box_dimensions, sf::Vector2f hit_box_relative_central_position, bool has_hb, bool clear_hit_enemies = false);
	void Update(bool facing_right);
	void Activate();
	void Deactivate();
	sf::Vector2f GetKnockBack();
	int GetHitStunFrames();
	bool HasHitBox();
	int GetDamage();
	bool IsClearingFrame();
};

class Attack {
private:
	sf::Uint64 current_frame_in_attack;
	sf::Uint64 current_frame;
	sf::Uint64 starting_attack_frame;
	std::vector<std::vector<AttackFrame*>> attack_frames;
	StatusTimer* attack_timer;
	b2Body* player_body;
	int player_index;
	std::vector<string> enemies_hit;
	std::vector<std::vector<AttackFrame*>> MakeMoveJab();
	std::vector<std::vector<AttackFrame*>> MakeMoveUpSmash();
	std::vector<std::vector<AttackFrame*>> MakeMoveForwardSmash();
	std::vector<std::vector<AttackFrame*>> MakeMoveDownAir();
	std::vector<std::vector<AttackFrame*>> MakeMoveForwardAir();
	std::vector<std::vector<AttackFrame*>> MakeMoveBackAir();
	std::vector<std::vector<AttackFrame*>> MakeMoveUpAir();
	std::vector<std::vector<AttackFrame*>> MakeMoveDownSmash();
	std::vector<std::vector<AttackFrame*>> MakeMoveNeutralAir();
public:
	Attack(b2Body* body, int index, int move_type);
	Attack(b2Body* body, int index, int move_type, Json::Value jsonData);
	void Update(sf::Uint64 curr_frame, bool facing_right);
	void InitiateAttack();
	int GetDamage();
	bool IsAttacking();
	sf::Vector2f GetKnockBack();
	int GetHitStunFrames();
	std::vector<std::vector<AttackFrame*>> Attack::GetAttackFrames(int move);
	bool CanHitTarget(string);
	void StopAttack();

	enum Moves {
		JAB = 0,
		UP_SMASH = 1,
		DOWN_SMASH = 2,
		FORWARD_SMASH = 3,
		UP_AIR = 4,
		DOWN_AIR = 5,
		FORWARD_AIR = 6,
		BACK_AIR = 7,
		NEUTRAL_AIR = 8,
		DASH_ATTACK = 9,
		DASH_PUNCH = 10,
		THROW_WEAPON = 11,
		TELEPORT_TO_WEAPON = 12,
		URIENS_SUPER = 13,
		MOVES_COUNT = 14
	};
};

#endif