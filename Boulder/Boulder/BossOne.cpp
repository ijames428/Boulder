#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "BossOne.h"
//#include "..\PlatformerLibrary\Test.h"

BossOne::BossOne(string unit_name, string unit_type, string bestiary_name, bool is_npc, Json::Value jsonBestiariesData, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	BoulderCreature::BoulderCreature(unit_name, unit_type, bestiary_name, is_npc, jsonBestiariesData, window, position, dimensions, subject_to_gravity) {
	jump_power = 15.0f;
	distance_from_target_last_frame = 10.0f;
	hovering = false;
}

void BossOne::UpdateBehavior() {
	if (target != nullptr)
	{
		float delta_x = target->GetBody()->GetPosition().x - body->GetPosition().x;
		float delta_y = target->GetBody()->GetPosition().y - body->GetPosition().y;

		float distance = sqrtf(powf(delta_x, 2) + powf(delta_y, 2));

		if (distance < 12.0f) {
			if (current_frame % 60 == 0 || (distance_from_target_last_frame >= 12.0f)) {
				int rng = rand() % 5;

				if (!IsAnAttackActive()) {
					if (target->GetBody()->GetPosition().x < body->GetPosition().x) {
						SetFacingRight(false);
					} else if (target->GetBody()->GetPosition().x > body->GetPosition().x) {
						SetFacingRight(true);
					}
				}

				if (IsInTheAir()) {
					if (rng >= 2) {
						if (!IsAnAttackActive()) {
							UseAttack(1);
						}
					} else {
						hovering = false;
					}
				} else {
					if (rng == 0) {
						if (target->GetBody()->GetPosition().x < body->GetPosition().x)
						{
							movement = -100.0f;
							running = false;
						}
						else if (target->GetBody()->GetPosition().x > body->GetPosition().x)
						{
							movement = 100.0f;
							running = false;
						}
					} else if (rng == 1) {
						movement = 0.0f;
						running = false;
					} else if (rng == 2) {
						if (!IsAnAttackActive()) {
							UseAttack(0);
						}
					} else if (rng >= 3) {
						Jump();
					}
				}
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

		distance_from_target_last_frame = distance;
	}
	else if (current_frame % 60 == 0) {
		int rng = rand() % 5;

		if (rng == 0) {
			movement = -100.0f;
			running = false;
		}
		else if (rng == 1) {
			movement = 0.0f;
			running = false;
		}
		else if (rng == 2) {
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

		if (hovering) {
			body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
		}

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
		} else if (IsInTheAir()) {
			if (body->GetLinearVelocity().y < -2.0f) {
				State = STATE_JUMPING;
			} else if (body->GetLinearVelocity().y > 0.0f) {
				State = STATE_FALLING;
			} else {
				hovering = true;
				State = STATE_JUMP_APEX;
			}
		} else if (body->GetLinearVelocity().x != 0) {
			if (running) {
				State = STATE_RUNNING;
			}
			else {
				State = STATE_WALKING;
			}
		} else {
			State = STATE_IDLE;
		}
	}
}

void BossOne::Deaggro() {
}

void BossOne::Land() {
	BoulderCreature::Land();

	hovering = false;
}