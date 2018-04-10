#pragma once

#ifndef PLAYERCHARACTER_H
#define PLAYERCHARACTER_H

using namespace std;
#include <SFML/Audio.hpp>
//#include <SFML/Graphics.hpp>
//#include "..\PlatformerLibrary\RigidBody.h"
#include "GameLibrary\SpriteAnimation.h"
#include "GameLibrary\Settings.h"
//#include "..\GameLibrary\AssetManager.h"
//#include "World.h"
#include "Creature.h"
#include "Projectile.h"
//#include "..\GameLibrary\Singleton.h"
#include "PlatformerLibrary\Box2D\Box2D.h"

class PlayerCharacter : public Creature {
	private:
		bool hit_terminal_velocity;
		sf::Color player_color;
		SpriteAnimation* running_animation;
		sf::Texture idle_texture;
		sf::Sprite idle_sprite;
		float idle_sprite_scale;
		sf::Texture attack_texture;
		sf::Sprite attack_sprite;
		sf::Texture fire_texture;
		sf::Sprite fire_sprite;
		sf::Texture dodge_texture;
		sf::Sprite dodge_sprite;
		sf::Texture taking_damage_texture;
		sf::Sprite taking_damage_sprite;
		sf::Texture dead_on_ground_texture;
		sf::Sprite dead_on_ground_sprite;
		std::vector<sf::Sound> hit_sounds = std::vector<sf::Sound>();
		sf::SoundBuffer buffer0;
		sf::SoundBuffer buffer1;
		sf::SoundBuffer buffer2;
		sf::Sound landing_sound;
		sf::SoundBuffer landing_buffer;
		sf::Sound sword_hitting_enemy_sound;
		sf::SoundBuffer sword_hitting_enemy_buffer;
		sf::Sound sword_whiffing_sound;
		sf::SoundBuffer sword_whiffing_buffer;
		sf::Sound firing_projectile_sound;
		sf::SoundBuffer firing_projectile_buffer;
		sf::Sound soundLand;
		sf::SoundBuffer bufferLand;
		sf::Sound soundJump;
		sf::SoundBuffer bufferJump;
		RigidBody* HitBox;
		bool was_in_air;
		sf::Int64 time_of_last_attack;
		sf::Int64 attack_cooldown;
		sf::Int64 attack_duration;
		sf::Int64 time_of_last_fire;
		sf::Int64 fire_cooldown;
		sf::Int64 fire_duration;
	public:
		b2BodyDef bodyDef;
		b2Body* body;
		b2PolygonShape dynamicBox;
		b2FixtureDef fixtureDef;
		int player_index;
		bool can_take_input;
		int max_hit_points;
		std::vector<Projectile*> projectiles = std::vector<Projectile*>();
		PlayerCharacter(sf::RenderWindow *window, sf::Vector2f position = sf::Vector2f(0.0f, 0.0f), sf::Vector2f dimensions = sf::Vector2f(0.0f, 0.0f), bool subject_to_gravity = true);
		void HandleLeftStickInput(float horizontal, float vertical);
		void HandleButtonAPress();
		void HandleButtonARelease();
		void HandleButtonBPress();
		void HandleButtonBRelease();
		void HandleButtonXPress();
		void HandleButtonXRelease();
		void HandleButtonYPress();
		void HandleButtonYRelease();
		void HandleButtonRightBumperPress();
		void HandleButtonRightBumperRelease();
		void HandleButtonLeftBumperPress();
		void HandleButtonLeftBumperRelease();
		void HandleButtonSelectPress();
		void HandleButtonSelectRelease();
		void HandleButtonStartPress();
		void HandleButtonStartRelease();
		void Draw(sf::Vector2f camera_position);
		virtual void Update(sf::Int64 curr_time, sf::Int64 delta_time);
};

#endif