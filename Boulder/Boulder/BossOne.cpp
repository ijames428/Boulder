#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "BossOne.h"
#include "SmashWorld.h"
//#include "..\PlatformerLibrary\Test.h"

BossOne::BossOne(Json::Value jsonUnitInMapData, Json::Value jsonBestiariesData, sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity) :
	BoulderCreature::BoulderCreature(jsonUnitInMapData, jsonBestiariesData, window, position, dimensions, subject_to_gravity) {
	jump_power = 15.0f;
	distance_from_target_last_frame = 10.0f;
	hovering = false;
	hitStunWasActiveLastFrame = false;
	attacksAreInterruptible = false;

	sf::Vector2f viewport = Singleton<SmashWorld>::Get()->GetCamera()->viewport_dimensions;
	viewport_width = viewport.x;
	viewport_height = viewport.y;
	starting_health_bar_width = viewport_width - 100.0f;

	healthBarBackgroundRect = new sf::RectangleShape(sf::Vector2f(starting_health_bar_width + 10.0f, 30.0f));
	healthBarBackgroundRect->setPosition(45.0f, viewport_height - 75.0f);
	healthBarBackgroundRect->setFillColor(sf::Color::Black);
}

void BossOne::Draw(sf::Vector2f camera_position) {
	if (target != nullptr && hit_points > 0) {
		render_window->draw(*healthBarBackgroundRect);
		float percent_health = ((float)hit_points / (float)max_hit_points);

		healthBarRect->setSize(sf::Vector2f(starting_health_bar_width * percent_health, 20.0f));
		healthBarRect->setPosition(sf::Vector2f(50.0f, viewport_height - 70.0f));
		healthBarRect->setFillColor(sf::Color((int)(0.0f + (255.0f - (float)hit_points / (float)max_hit_points) * 255.0f), (int)((float)hit_points / (float)max_hit_points * 255.0f), 0, 255));

		float percent_cash_in_health = ((float)cashinable_hit_point_value / (float)max_hit_points);

		cashInHealthBarRect->setSize(sf::Vector2f(starting_health_bar_width * percent_cash_in_health, 20.0f));
		cashInHealthBarRect->setPosition(sf::Vector2f(50.0f, viewport_height - 70.0f));

		render_window->draw(*cashInHealthBarRect);
		render_window->draw(*healthBarRect);
	}

	BoulderCreature::Draw(camera_position);
}

void BossOne::UpdateBehavior() {
	if (target != nullptr)
	{
		float delta_x = target->GetBody()->GetPosition().x - body->GetPosition().x;
		float delta_y = target->GetBody()->GetPosition().y - body->GetPosition().y;

		float distance = sqrtf(powf(delta_x, 2) + powf(delta_y, 2));

		if (distance < 12.0f) {
			if (current_frame % 60 == 0 || (distance_from_target_last_frame >= 12.0f) || (hitStunWasActiveLastFrame && !hit_stun_timer->IsActive())) {
				int rng = 2;// rand() % 5;

				if (!IsAnAttackActive() && hit_points > 0) {
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
							movementX = -100.0f;
							running = false;
						}
						else if (target->GetBody()->GetPosition().x > body->GetPosition().x)
						{
							movementX = 100.0f;
							running = false;
						}
					} else if (rng == 1) {
						movementX = 0.0f;
						running = false;
					} else if (rng == 2) {
						if (!IsAnAttackActive()) {
							UseAttack(0);
						}
					} else if (rng >= 3) {
						ActuallyJump();
					}
				}
			}
		} else {
			if (target->GetBody()->GetPosition().x < body->GetPosition().x)
			{
				movementX = -100.0f;
				running = true;
			}
			else if (target->GetBody()->GetPosition().x > body->GetPosition().x)
			{
				movementX = 100.0f;
				running = true;
			}
		}

		distance_from_target_last_frame = distance;
	} else if (current_frame % 60 == 0) {
		int rng = rand() % 5;

		if (rng == 0) {
			movementX = -100.0f;
			running = false;
		}
		else if (rng == 1) {
			movementX = 0.0f;
			running = false;
		}
		else if (rng == 2) {
			movementX = 100.0f;
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
		Move(movementX, 0.0f);

		if (hovering && !hit_stun_timer->IsActive()) {
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

	hitStunWasActiveLastFrame = hit_stun_timer->IsActive();
}

void BossOne::Deaggro() {
}

void BossOne::Land() {
	BoulderCreature::Land();

	hovering = false;
}