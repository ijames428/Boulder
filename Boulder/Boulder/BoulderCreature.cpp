#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "BoulderCreature.h"
#include "Constants.h"
#include "SmashWorld.h"
#include <cmath>

BoulderCreature::BoulderCreature(int player_idx, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	Creature::Creature(window, position, dimensions, subject_to_gravity) {
}

BoulderCreature::BoulderCreature(string unit_name, string unit_type, string bestiary_name, bool is_npc, Json::Value jsonBestiariesData, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	Creature::Creature(window, position, dimensions, subject_to_gravity) {
	SetEntityType(Constants::ENTITY_TYPE_CREATURE);
	name = unit_name;
	type = unit_type;
	movement = 0.0f;
	SetFacingRight(true);
	player_index = 1;

	hit_points = max_hit_points = jsonBestiariesData["DictOfUnits"][unit_type]["HitPoints"].asInt();
	interaction_radius = jsonBestiariesData["DictOfUnits"][unit_type]["InteractionRadius"].asFloat();
	can_take_input = false;

	speed = jsonBestiariesData["DictOfUnits"][unit_type]["MovementSpeed"].asFloat();
	running_speed_multiplier = 3.0f;
	jump_power = 10.0f;

	is_interactable = is_npc;
	is_hittable = !is_interactable;
	is_hostile = !is_interactable;

	sf::RectangleShape shape(dimensions);
	shape.setFillColor(sf::Color::Yellow);
	shape.setPosition(position);
	rectangle_shape = shape;
	player_color = sf::Color::Cyan;

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(position.x, position.y);
	body = Singleton<SmashWorld>::Get()->GetB2World()->CreateBody(&bodyDef);

	aggroCircleShape.m_p.Set(0, 0); //position, relative to body position
	aggroCircleShape.m_radius = 3.0f;
	aggroCircleFixtureDef.shape = &aggroCircleShape;
	aggroCircleFixtureDef.isSensor = true;
	aggroCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	aggroCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->AGGRO_CIRCLE;
	aggroCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;

	deaggroCircleShape.m_p.Set(0, 0); //position, relative to body position
	deaggroCircleShape.m_radius = 5.0f;
	deaggroCircleFixtureDef.shape = &deaggroCircleShape;
	deaggroCircleFixtureDef.isSensor = true;
	deaggroCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 1.0f, 1.0f);
	deaggroCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->DEAGGRO_CIRCLE;
	deaggroCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;

	interactionCircleShape.m_p.Set(0, 0); //position, relative to body position
	interactionCircleShape.m_radius = interaction_radius / 100.0f;
	interactionCircleFixtureDef.shape = &interactionCircleShape;
	interactionCircleFixtureDef.isSensor = true;
	interactionCircleFixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	interactionCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->INTERACTION_CIRCLE;
	interactionCircleFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLAYER_ONE;

	centerBoxShape.SetAsBox(dimensions.x / 4.0f - 0.01f, 0.5f);
	topCircleShape.m_p.Set(0, -0.5f); //position, relative to body position
	botCircleShape.m_p.Set(0, 0.5f); //position, relative to body position
	topCircleShape.m_radius = dimensions.x / 4.0f;
	botCircleShape.m_radius = dimensions.x / 4.0f;
	centerBoxFixtureDef.shape = &centerBoxShape;
	centerBoxFixtureDef.density = 1.0f;
	centerBoxFixtureDef.friction = 0.0f;
	centerBoxFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	topCircleFixtureDef.shape = &topCircleShape;
	topCircleFixtureDef.density = 1.0f;
	topCircleFixtureDef.friction = 0.01f;
	topCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);
	botCircleFixtureDef.shape = &botCircleShape;
	botCircleFixtureDef.density = 1.0f;
	botCircleFixtureDef.friction = 0.3f;
	botCircleFixtureDef.m_color = new b2Color(0.0f, 1.0f, 0.0f, 1.0f);

	topCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	botCircleFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;
	centerBoxFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLAYER_TWO;

	aggroCircleFixture = body->CreateFixture(&aggroCircleFixtureDef);
	deaggroCircleFixture = body->CreateFixture(&deaggroCircleFixtureDef);
	interactionCircleFixture = body->CreateFixture(&interactionCircleFixtureDef);
	topCircleFixture = body->CreateFixture(&topCircleFixtureDef);
	botCircleFixture = body->CreateFixture(&botCircleFixtureDef);
	centerBoxFixture = body->CreateFixture(&centerBoxFixtureDef);

	//interactionCircleFixture->SetActive(is_interactable && interaction_radius > 0.0f);

	if (is_interactable) {
		topCircleFixture->SetSensor(true);
		botCircleFixture->SetSensor(true);
		centerBoxFixture->SetSensor(true);

		body->SetGravityScale(0.0f);
	}

	body->SetFixedRotation(true);
	body->SetUserData(this);

	groundCheckShape.SetAsBox(dimensions.x / 2.0f, 0.05f, b2Vec2(0.0f, 0.6f), 0.0f);
	groundCheckFixtureDef.shape = &groundCheckShape;
	groundCheckFixtureDef.isSensor = true;
	groundCheckFixtureDef.m_color = new b2Color(1.0f, 0.0f, 1.0f, 1.0f);

	groundCheckFixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->GROUND_CHECK;
	groundCheckFixtureDef.filter.maskBits = Singleton<SmashWorld>::Get()->PLATFORM;

	body->CreateFixture(&groundCheckFixtureDef);

	attacks.push_back(new Attack(body, player_index, Attack::FORWARD_SMASH, jsonBestiariesData["DictOfUnits"][unit_type]["AttackingAnimations"][0]));
	attacks.push_back(new Attack(body, player_index, Attack::DOWN_SMASH, jsonBestiariesData["DictOfUnits"][unit_type]["AttackingAnimations"][1]));

	sprite_scale = 0.3f;

	LoadAllAnimations(unit_type, jsonBestiariesData);

	hit_stun_timer = new StatusTimer(1);
	jump_input_buffer = new StatusTimer(6);

	int numberOfDyingAnimationFrames = dying_animations.size() > 0 ? dying_animations[0]->GetNumberOfFrames() : 1;
	dying_animation_timer = new StatusTimer(numberOfDyingAnimationFrames);

	int numberOfLandingAnimationFrames = landing_animations.size() > 0 ? landing_animations[0]->GetNumberOfFrames() : 1;
	landing_animation_timer = new StatusTimer(numberOfLandingAnimationFrames);

	talking_animation_timer = new StatusTimer(120);
}

void BoulderCreature::ApplyObjectDataToSaveData(Json::Value& save_data) {
	//save_data["Name"] = name;
	save_data["PositionX"] = body->GetPosition().x;
	save_data["PositionY"] = body->GetPosition().y;
}

void BoulderCreature::ApplySaveDataToObjectData(Json::Value& save_data) {
	body->SetTransform(b2Vec2(save_data["PositionX"].asFloat(), save_data["PositionY"].asFloat()), body->GetAngle());
}

void BoulderCreature::LoadAllAnimations(string unit_type, Json::Value jsonBestiariesData) {
	idle_animations = LoadAnimations("IdleAnimations", unit_type, jsonBestiariesData);
	walking_animations = LoadAnimations("WalkingAnimations", unit_type, jsonBestiariesData);
	running_animations = LoadAnimations("RunningAnimations", unit_type, jsonBestiariesData);
	dying_animations = LoadAnimations("DyingAnimations", unit_type, jsonBestiariesData);
	dead_animations = LoadAnimations("DeadAnimations", unit_type, jsonBestiariesData);
	attacking_animations = LoadAnimations("AttackingAnimations", unit_type, jsonBestiariesData);
	blocking_animations = LoadAnimations("BlockingAnimations", unit_type, jsonBestiariesData);
	hit_stun_animations = LoadAnimations("HitStunAnimations", unit_type, jsonBestiariesData);
	jumping_animations = LoadAnimations("JumpingAnimations", unit_type, jsonBestiariesData);
	jump_apex_animations = LoadAnimations("JumpApexAnimations", unit_type, jsonBestiariesData);
	falling_animations = LoadAnimations("FallingAnimations", unit_type, jsonBestiariesData);
	landing_animations = LoadAnimations("LandingAnimations", unit_type, jsonBestiariesData);
	talking_animations = LoadAnimations("TalkingAnimations", unit_type, jsonBestiariesData);
}

std::vector<SpriteAnimation*> BoulderCreature::LoadAnimations(string animations_name, string unit_type, Json::Value jsonData) {
	std::vector<SpriteAnimation*> animations = std::vector<SpriteAnimation*>();
	int number_of_animations = (int)jsonData["DictOfUnits"][unit_type][animations_name].size();

	for (int i = 0; i < number_of_animations; i++)
	{
		string filePath = jsonData["DictOfUnits"][unit_type][animations_name][i]["FilePath"].asString();

		if (filePath != "") {
			size_t findResult = filePath.find("Units\\");
			string relativeFilePath = filePath.substr(findResult);

			animations.push_back(new SpriteAnimation(render_window,
				relativeFilePath,
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FrameDimensionsX"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FrameDimensionsY"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["NumberOfFrames"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FramesPerColumn"].asInt(),
				jsonData["DictOfUnits"][unit_type][animations_name][i]["FramesPerRow"].asInt(),
				sprite_scale, sf::Color::White, animations_name != "JumpingAnimations" && animations_name != "JumpApexAnimations"));
		}
	}

	return animations;
}

void BoulderCreature::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	Creature::Update(curr_frame, delta_time);

	can_take_input = !hit_stun_timer->IsActive();
	if (can_take_input) {
		can_take_input = !IsAnAttackActive();
	}

	if (body->GetLinearVelocity().x > 0.1f && !IsFacingRight()) {
		SetFacingRight(true);
	}
	else if (body->GetLinearVelocity().x < -0.1f && IsFacingRight()) {
		SetFacingRight(false);
	}

	for (int i = 0; i < (int)attacks.size(); i++) {
		attacks[i]->Update(curr_frame, IsFacingRight());
	}

	if (hit_stun_timer->IsActive()) {
		topCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else {
		topCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		botCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
		centerBoxFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
	}

	if (target != nullptr)
	{
		float delta_x = target->GetBody()->GetPosition().x - body->GetPosition().x;
		float delta_y = target->GetBody()->GetPosition().y - body->GetPosition().y;

		float distance = sqrtf(powf(delta_x, 2) + powf(delta_y, 2));

		if (distance < 2.0f) {
			if (!IsAnAttackActive()) {
				UseAttack(1);
			}
		} else {
			if (target->GetBody()->GetPosition().x < body->GetPosition().x)
			{
				movement = -100.0f;
				running = true;
			}
			else if (target->GetBody()->GetPosition().x > body->GetPosition().x)
			{
				movement = 100.0f;
				running = true;
			}
		}
	} else if (curr_frame % 60 == 0) {
		int rng = rand() % 5;

		if (rng == 0) {
			movement = -100.0f;
			running = false;
		} else if (rng == 1) {
			movement = 0.0f;
			running = false;
		} else if (rng == 2) {
			movement = 100.0f;
			running = false;
		}
	}

	if (is_interactable) {
		body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));

		if (talking_animation_timer->IsActive()) {
			State = STATE_TALKING;
		} else {
			State = STATE_IDLE;
		}
	} else {
		Move(movement, 0.0f);

		if (hit_points <= 0) {
			if (dying_animation_timer->IsActive()) {
				State = STATE_DYING;
			} else {
				State = STATE_DEAD;
			}
		} else if (hit_stun_timer->IsActive()) {
			State = STATE_HIT_STUN;
		} else if (IsAnAttackActive()) {
			State = STATE_ATTACKING;
		} else if (body->GetLinearVelocity().x != 0) {
			if (running) {
				State = STATE_RUNNING;
			} else {
				State = STATE_WALKING;
			}
		} else {
			State = STATE_IDLE;
		}
	}
}

void BoulderCreature::Draw(sf::Vector2f camera_position) {
	//if (body->GetLinearVelocity().x > 0 && !IsFacingRight()) {
	//	SetFacingRight(true);
	//} else if (body->GetLinearVelocity().x < 0 && IsFacingRight()) {
	//	SetFacingRight(false);
	//}
	FlipAnimationsIfNecessary(idle_animations);
	FlipAnimationsIfNecessary(walking_animations);
	FlipAnimationsIfNecessary(running_animations);
	FlipAnimationsIfNecessary(dying_animations);
	FlipAnimationsIfNecessary(dead_animations);
	FlipAnimationsIfNecessary(attacking_animations);
	FlipAnimationsIfNecessary(blocking_animations);
	FlipAnimationsIfNecessary(hit_stun_animations);
	FlipAnimationsIfNecessary(jumping_animations);
	FlipAnimationsIfNecessary(jump_apex_animations);
	FlipAnimationsIfNecessary(falling_animations);
	FlipAnimationsIfNecessary(landing_animations);
	FlipAnimationsIfNecessary(talking_animations);

	if (State == STATE_IDLE) {
		idle_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	}else if (State == STATE_WALKING) {
		walking_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	} else  if (State == STATE_RUNNING) {
		running_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	} else if (State == STATE_DYING) {
		if ((int)dying_animations.size() > 0) {
			dying_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
		}
	} else if (State == STATE_DEAD) {
		if ((int)dead_animations.size() > 0) {
			dead_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
		}
	} else if (State == STATE_ATTACKING) {
		int currentAttackIndex = GetActiveAttackIndex();
		if (currentAttackIndex < (int)attacking_animations.size()) {
			attacking_animations[currentAttackIndex]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
		} else {
			attacking_animations[1]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
		}
	} else if (State == STATE_BLOCKING) {
		blocking_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	} else if (State == STATE_HIT_STUN) {
		hit_stun_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	} else if (State == STATE_JUMPING) {
		jumping_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	} else if (State == STATE_JUMP_APEX) {
		jump_apex_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	} else if (State == STATE_FALLING) {
		falling_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
	} else if (State == STATE_LANDING) {
		if ((int)landing_animations.size() > 0) {
			landing_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
		}
	} else if (State == STATE_TALKING) {
		if ((int)talking_animations.size() > 0) {
			talking_animations[0]->Draw(camera_position, sf::Vector2f((body->GetPosition().x), (body->GetPosition().y)));
		}
	}
}

void BoulderCreature::FlipAnimationsIfNecessary(std::vector<SpriteAnimation*> animations)
{
	for (int i = 0; i < (int)animations.size(); i++) {
		if (IsFacingRight() != animations[i]->IsFacingRight()) {
			animations[i]->Flip();
		}
	}
}

void BoulderCreature::Move(float horizontal, float vertical) {
	if (can_take_input && hit_points > 0) {
		body->SetLinearVelocity(b2Vec2((horizontal / 100.0f) * speed * (running ? running_speed_multiplier : 1.0f), body->GetLinearVelocity().y));

		if (horizontal > 0) {
			SetFacingRight(true);
		}
		else if (horizontal < 0) {
			SetFacingRight(false);
		}
	}
}

void BoulderCreature::Jump() {
	if (can_take_input && hit_points > 0) {
		bool jumping = false;

		if (!IsInTheAir()) {
			jumping = true;
		}
		else if (has_double_jump) {
			jumping = true;
			has_double_jump = false;
		}

		if (jumping) {
			body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x, -jump_power));
			SetInTheAir(true);
		}
	}
}

void BoulderCreature::Land() {
	landing_animation_timer->Start();
	SetInTheAir(false);
	has_double_jump = true;
	if (IsAnAttackActive()) {
		GetActiveAttack()->StopAttack();
	}
	if (jump_input_buffer->IsActive()) {
		Jump();
	}
}

void BoulderCreature::Aggro(BoulderCreature* new_target) {
	if (is_hostile) {
		aggroCircleFixture->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		deaggroCircleFixture->SetColor(0.0f, 1.0f, 0.0f, 1.0f);

		target = new_target;
	}
}

void BoulderCreature::Deaggro() {
	if (is_hostile) {
		aggroCircleFixture->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		deaggroCircleFixture->SetColor(1.0f, 1.0f, 1.0f, 1.0f);

		target = nullptr;
	}
}

void BoulderCreature::SetInteractable(BoulderCreature* new_interactable) {
	if (interactable == nullptr || new_interactable == nullptr) {
		interactable = new_interactable;
	}
}

void BoulderCreature::UseAttack(int move_type) {
	if (can_take_input && hit_points > 0) {
		attacks[move_type]->InitiateAttack();

		if (move_type < (int)attacking_animations.size()) {
			attacking_animations[move_type]->Play();
		} else {
			attacking_animations[1]->Play();
		}
	}
}

void BoulderCreature::TakeDamage(int damage, sf::Vector2f knock_back, int hit_stun_frames) {
	if (hit_points > 0 && is_hittable) {
		cout << "got hit for " << damage << " damage and " << hit_stun_frames << " hit stun frames!\n";
		hit_stun_timer = new StatusTimer(hit_stun_frames);
		hit_stun_timer->Start();
		body->SetLinearVelocity(b2Vec2(knock_back.x, knock_back.y));

		hit_points -= damage;

		if (hit_points <= 0)
		{
			hit_points = 0;
			dying_animation_timer->Start();
		}
	}
}

Attack* BoulderCreature::GetActiveAttack() {
	Attack* active_attack = attacks[0];

	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			active_attack = attacks[i];
		}
	}

	return active_attack;
}

int BoulderCreature::GetActiveAttackIndex() {
	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			return i;
		}
	}

	return 0;
}

bool BoulderCreature::IsAnAttackActive() {
	for (int i = 0; i < (int)attacks.size(); i++) {
		if (attacks[i]->IsAttacking()) {
			return true;
		}
	}

	return false;
}

void BoulderCreature::StartTalking() {
	talking_animation_timer->Start();
}