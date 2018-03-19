#pragma once

#ifndef BOULDER_CHARACTER_H
#define BOULDER_CHARACTER_H

using namespace std;
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
//#include "..\PlatformerLibrary\RigidBody.h"
#include "GameLibrary\SpriteAnimation.h"
#include "GameLibrary\Settings.h"
//#include "..\GameLibrary\AssetManager.h"
//#include "World.h"
#include "Creature.h"
#include "Projectile.h"
#include "PlatformerLibrary\Box2D\Box2D.h"
#include "Attack.h"
#include "StatusTimer.h"
#include "GameLibrary\Json\json.h"
#include "HurtBox.h"
#include "BoulderProjectile.h"

struct ProjectileFiringFrame {
	int attack_animation_index;
	int firing_frame;
	float relative_spawning_position_x;
	float relative_spawning_position_y;
};

class BoulderCreature : public Creature {
private:

protected:
	int State = 0;
	const int STATE_IDLE = 0;
	const int STATE_WALKING = 1;
	const int STATE_RUNNING = 2;
	const int STATE_DYING = 3;
	const int STATE_DEAD = 4;
	const int STATE_ATTACKING = 5;
	const int STATE_BLOCKING = 6;
	const int STATE_HIT_STUN = 7;
	const int STATE_JUMPING = 8;
	const int STATE_JUMP_APEX = 9;
	const int STATE_FALLING = 10;
	const int STATE_LANDING = 11;
	const int STATE_TALKING = 12;

	BoulderCreature* target = nullptr;

	bool is_hostile = false;
	bool is_interactable = false;
	bool is_hittable = false;
	float interaction_radius = 0.0f;

	BoulderCreature* interactable = nullptr;

	int GetActiveAttackIndex();
	float movement;
	bool running;
	float running_speed_multiplier;
	BoulderCreature(int player_idx, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	std::vector<Attack*> attacks = std::vector<Attack*>();
	int player_index;
	bool can_take_input;
	sf::Color player_color;
	b2BodyDef bodyDef;
	b2Body* body;
	b2CircleShape aggroCircleShape;
	b2CircleShape deaggroCircleShape;
	b2CircleShape interactionCircleShape;
	b2CircleShape topCircleShape;
	b2CircleShape botCircleShape;
	b2PolygonShape centerBoxShape;
	b2FixtureDef aggroCircleFixtureDef;
	b2FixtureDef deaggroCircleFixtureDef;
	b2FixtureDef interactionCircleFixtureDef;
	b2FixtureDef topCircleFixtureDef;
	b2FixtureDef botCircleFixtureDef;
	b2FixtureDef centerBoxFixtureDef;
	b2FixtureDef hurtBoxFixtureDef;
	b2Fixture* aggroCircleFixture;
	b2Fixture* deaggroCircleFixture;
	b2Fixture* interactionCircleFixture;
	b2Fixture* topCircleFixture;
	b2Fixture* botCircleFixture;
	b2Fixture* centerBoxFixture;
	b2Fixture* hurtBoxFixture;
	b2PolygonShape groundCheckShape;
	b2FixtureDef groundCheckFixtureDef;
	bool has_double_jump;
	void Move(float horizontal, float vertical);
	bool IsJumping();
	void StartJump();
	virtual void ActuallyJump(bool short_hop = false);
	void UseAttack(int move_type, bool activate_buffer = true);
	sf::RectangleShape* healthBarRect;
	float starting_health_bar_width;
	string name = "";
	string type = "";

	void LoadAllAnimations(string unit_type, Json::Value jsonBestiariesData);
	std::vector<SpriteAnimation*> LoadAnimations(string animations_name, string unit_type, Json::Value jsonBestiariesData); 
	void FlipAnimationsIfNecessary(std::vector<SpriteAnimation*> animations);

	float sprite_scale;
	std::vector<SpriteAnimation*> idle_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> walking_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> running_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> dying_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> dead_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> attacking_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> blocking_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> hit_stun_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> jumping_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> jump_apex_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> falling_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> landing_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> talking_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> projectile_active_animations = std::vector<SpriteAnimation*>();
	std::vector<SpriteAnimation*> projectile_hit_animations = std::vector<SpriteAnimation*>();

	std::vector<ProjectileFiringFrame> projectile_firing_frames = std::vector<ProjectileFiringFrame>();

	StatusTimer* hit_stun_timer;
	StatusTimer* dying_animation_timer;
	StatusTimer* landing_animation_timer;
	StatusTimer* talking_animation_timer;
	StatusTimer* jumpStartUpTimer;
	StatusTimer* jumpInputBuffer;

	bool jumpInputBufferWasActiveLastFrame;
	bool jumpStartUpTimerWasActiveLastFrame;
	bool releasedJumpButton;

	StatusTimer* attack_input_buffer;
	int attack_buffer_attack_index;
	bool anAttackWasActiveLastFrame;

	StatusTimer* health_cash_in_timer;
	sf::Int16 cashinable_hit_point_value;
	sf::Int16 cashinable_hit_point_drain_rate;
	sf::RectangleShape* cashInHealthBarRect;

	std::vector<int> AttackAnimationSoundFrames = std::vector<int>();
	std::vector<sf::SoundBuffer*> AttackAnimationSoundBuffers = std::vector<sf::SoundBuffer*>();
	std::vector<sf::Sound*> AttackAnimationSounds = std::vector<sf::Sound*>();

	std::vector<sf::SoundBuffer*> GettingHitSoundBuffers = std::vector<sf::SoundBuffer*>();
	std::vector<sf::Sound*> GettingHitSounds = std::vector<sf::Sound*>();

	sf::SoundBuffer RightFootStepSoundBuffer;
	sf::SoundBuffer LeftFootStepSoundBuffer;
	sf::Sound RightFootStepSound;
	sf::Sound LeftFootStepSound;
	int RightFootStepSoundFrameRun;
	int LeftFootStepSoundFrameRun;
	int RightFootStepSoundFrameWalk;
	int LeftFootStepSoundFrameWalk;
	
	std::vector<string> activities;

	HurtBox* hurt_box;
	uint16 default_masked_bits;

	std::vector<BoulderProjectile*> projectiles = std::vector<BoulderProjectile*>();

	sf::Texture* interaction_button_texture;
	sf::Sprite* interaction_button_sprite;
	bool draw_interaction_button;
	float current_y_offset = 0.0f;

	Json::Value unit_type_json_data;
	int attacks_size;

	float maxVerticalVelocityReached = 0.0f;
	//float xVelocityWhenJumpHappened = 0.0f;
	float maxAirSpeed = 0.0f;
	float normalTerminalVelocity = 20.0f;
	float fastFallingVelocity = 40.0f;
	bool fastFalling = false;

	float attackDistance = 0.0f;

	float knockBackMultiplier = 1.0f;
	float knockBackMultiplierIncreasePerHit = 0.3f;

	float hitStunMultiplier = 1.0f;
	float hitStunMultiplierDecreasePerHit = 0.1f;
	float minimumHitStunMultiplier = 0.2f;

	bool attacksAreInterruptible;

	int platformContacts = 0;
public:
	BoulderCreature(string unit_name, string unit_type, string bestiary_name, bool is_npc, Json::Value jsonBestiariesData, sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
	void Draw(sf::Vector2f camera_position);
	virtual void Update(sf::Int64 curr_frame, sf::Int64 delta_time);
	virtual void TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies);
	virtual void ReceiveHeal(int heal);
	int TakeDamageWithLifeSteal(int damage, sf::Vector2f knock_back, int hit_stun_frames, bool pop_up_grounded_enemies);
	Attack* GetActiveAttack();
	bool IsAnAttackActive();
	virtual void Land();
	void AddPlatformContact();
	void RemovePlatformContact();
	void Aggro(BoulderCreature* new_target);
	virtual void Deaggro();
	void SetInteractable(BoulderCreature* new_interactable);
	void StartTalking();
	virtual void ApplyObjectDataToSaveData(Json::Value& save_data);
	virtual void ApplySaveDataToObjectData(Json::Value& save_data);
	void AddActivaty(string activity);
	virtual void UpdateBehavior(); 
	bool IfShouldUpdate(sf::Vector2f player_screen_pos, sf::Vector2f viewport_dimensions); 
	void ReverseHorizontalDirectionIfInHitStun();
	virtual int GetDamageOfCurrentAttack();
	virtual void UpdateEffectsVolumes(float new_effects_volume);
	virtual void AddAnger(int anger_amount);
	bool IsInHitStun();

	string GetName() {
		return name;
	};
	void SetName(string new_name) {
		name = new_name;
	};
	string GetType() {
		return type;
	};
	b2Body* GetBody() {
		return body;
	}
	sf::Vector2f GetScreenPosition() {
		return sf::Vector2f(body->GetPosition().x * 40.0f, body->GetPosition().y * 40.0f);
	}
	virtual int GetHitPoints() {
		return hit_points;
	};
};

#endif