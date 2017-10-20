#pragma once

#include "stdafx.h"
#include <iostream>
#include "RigidBody.h"
#include "GridHandler.h"
#include "..\GameLibrary\Singleton.h"
#include "..\Constants.h"

RigidBody::RigidBody(sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity, bool subject_to_collision) {
	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;

	future_position.x = past_position.x = current_position.x = position.x;
	future_position.y = past_position.y = current_position.y = position.y;
	future_dimensions.x = past_dimensions.x = current_dimensions.x = dimensions.x;
	future_dimensions.y = past_dimensions.y = current_dimensions.y = dimensions.y;

	gravity_enabled = subject_to_gravity;
	collision_enabled = subject_to_collision;
	facing_right = true;
	only_collide_with_platforms = false;

	weight = 10.0f;

	gravity_acceleration = 0.5f;
	terminal_velocity = 30.0f;

	velocity.x = 0.0f;
	velocity.y = 0.0f;

	entities_excluded_from_collision = std::vector<int>();
	Singleton<GridHandler>::Get()->AddRigidBodyToGrid(this);
}

void RigidBody::Update(sf::Int64 curr_time, sf::Int64 delta_time) {
	float mMovementSpeedTimefactor = delta_time / 10.0f;

	if (mMovementSpeedTimefactor > 2.0f) {
		mMovementSpeedTimefactor = 2.0f;
	}

	future_dimensions.x = current_dimensions.x;
	future_dimensions.y = current_dimensions.y;

	if (future_dimensions.y < past_dimensions.y) {
		current_position.y += past_dimensions.y - future_dimensions.y;
	}
	else if (future_dimensions.y > past_dimensions.y) {
		current_position.y -= future_dimensions.y - past_dimensions.y;
	}

	future_position.x = current_position.x;
	future_position.y = current_position.y;

	float pregravity_velocity_y = velocity.y;
	in_the_air = true;

	if (gravity_enabled) {
		velocity.y = velocity.y + gravity_acceleration;// * mMovementSpeedTimefactor;
	}

	if (velocity.y > terminal_velocity) {
		velocity.y = terminal_velocity;
	}

	if (velocity.x > 0.0f) {
		facing_right = true;

		if (pregravity_velocity_y == 0.0f && future_position.y == past_position.y) {
			if (velocity.x > 0.2f) {
				velocity.x = velocity.x - 0.2f;
			}
			else {
				velocity.x = 0.0f;
			}
		}
	}
	else if (velocity.x < 0.0f) {
		if (pregravity_velocity_y == 0.0f && future_position.y == past_position.y) {
			facing_right = false;

			if (velocity.x < -0.2f) {
				velocity.x = velocity.x + 0.2f;
			}
			else {
				velocity.x = 0.0f;
			}
		}
	}

	if (lock_facing_direction_when_hit) {
		facing_right = facing_right_when_hit;
	}

	future_position.x = current_position.x + velocity.x;// * mMovementSpeedTimefactor;
	if (entity_type == Constants::ENTITY_TYPE_PLAYER_CHARACTER) {
		future_position.y = current_position.y + velocity.y;// * mMovementSpeedTimefactor;
	}
	else {
		future_position.y = current_position.y + velocity.y;// * mMovementSpeedTimefactor;
	}

	if (past_position.x != future_position.x || past_position.y != future_position.y || past_dimensions.x != future_dimensions.x || past_dimensions.y != future_dimensions.y)
	{
		Singleton<GridHandler>::Get()->MoveRigidBodyInGrid(this);

		if (collision_enabled) {
			ChangeFutureValuesAndVelocityBasedOnCollisions();
		}

		current_position.x = future_position.x;
		current_position.y = future_position.y;
		current_dimensions.x = future_dimensions.x;
		current_dimensions.y = future_dimensions.y;
	}

	past_position.x = current_position.x;
	past_position.y = current_position.y;
	past_dimensions.x = current_dimensions.x;
	past_dimensions.y = current_dimensions.y;
}

void RigidBody::ChangeFutureValuesAndVelocityBasedOnCollisions() {
	for (int w = grid_top_left.x; w <= grid_bot_right.x; w++) {
		for (int h = grid_top_left.y; h <= grid_bot_right.y; h++) {
			std::vector<RigidBody*> colliders = Singleton<GridHandler>::Get()->GetObjectsInGridLocation(w, h);

			if (colliders.size() > 1) {
				for (int c = 0; c < (int)colliders.size(); c++) {
					if (id != colliders[c]->id && colliders[c]->collision_enabled &&
						!colliders[c]->only_collide_with_platforms &&
						(std::find(entities_excluded_from_collision.begin(), entities_excluded_from_collision.end(), colliders[c]->entity_type) == entities_excluded_from_collision.end())) {
						bool horizontal_collision = AreTheRigidBodiesCollidingHorizontally(this, colliders[c]);
						bool vertical_collision = AreTheRigidBodiesCollidingVertically(this, colliders[c]);

						if (horizontal_collision) {
							if (current_position.x + (current_dimensions.x / 2.0f) < colliders[c]->current_position.x + (colliders[c]->current_dimensions.x / 2.0f)) {
								future_position.x = current_position.x + (colliders[c]->current_position.x - (current_position.x + future_dimensions.x));
							}
							else {
								future_position.x = current_position.x - (current_position.x - (colliders[c]->current_position.x + colliders[c]->current_dimensions.x));
							}

							//if (abs(future_position.x - x) > abs(velocity.x)) {
							//	future_position.x = x;
							//}

							velocity.x = 0.0f;
						}

						if (vertical_collision) {
							if (current_position.y + (current_dimensions.y / 2.0f) < colliders[c]->current_position.y + (colliders[c]->current_dimensions.y / 2.0f)) {
								future_position.y = current_position.y + (colliders[c]->current_position.y - (current_position.y + future_dimensions.y));
								in_the_air = false;
							}
							else {
								future_position.y = current_position.y - (current_position.y - (colliders[c]->current_position.y + colliders[c]->current_dimensions.y));
							}

							//if (abs(future_position.y - y) > abs(velocity.y)) {
							//	future_position.y = y;
							//}

							velocity.y = 0.0f;
						}
					}
				}
			}
		}
	}
}

bool RigidBody::AreTheRigidBodiesCollidingHorizontally(RigidBody* rb1, RigidBody* rb2, bool just_check_no_response) {
	bool rb1_is_horizontally_outside_rb2 = (rb1->future_position.x + rb1->future_dimensions.x < rb2->current_position.x || rb1->future_position.x > rb2->current_position.x + rb2->current_dimensions.x);
	bool rb1_is_vertically_outside_rb2 = false;
	if (just_check_no_response) {
		rb1_is_vertically_outside_rb2 = (rb1->current_position.y + rb1->future_dimensions.y < rb2->current_position.y || rb1->current_position.y > rb2->current_position.y + rb2->current_dimensions.y);
	}
	else {
		rb1_is_vertically_outside_rb2 = ((rb1->current_position.y + 5.0f) + (rb1->future_dimensions.y - 10.0f) < rb2->current_position.y || rb1->current_position.y + 5.0f > rb2->current_position.y + rb2->current_dimensions.y);
	}

	if (rb1_is_horizontally_outside_rb2 || rb1_is_vertically_outside_rb2) {
		return false;
	}

	return true;
}

bool RigidBody::AreTheRigidBodiesCollidingVertically(RigidBody* rb1, RigidBody* rb2, bool just_check_no_response) {
	bool rb1_is_horizontally_outside_rb2 = false;
	bool rb1_is_vertically_outside_rb2 = (rb1->future_position.y + rb1->future_dimensions.y < rb2->current_position.y || rb1->future_position.y > rb2->current_position.y + rb2->current_dimensions.y);

	if (just_check_no_response) {
		rb1_is_horizontally_outside_rb2 = (rb1->current_position.x + rb1->future_dimensions.x < rb2->current_position.x || rb1->current_position.x > rb2->current_position.x + rb2->current_dimensions.x);
	}
	else {
		rb1_is_horizontally_outside_rb2 = ((rb1->current_position.x + 5.0f) + (rb1->future_dimensions.x - 10.0f) < rb2->current_position.x || rb1->current_position.x + 5.0f > rb2->current_position.x + rb2->current_dimensions.x);
	}

	if (rb1_is_horizontally_outside_rb2 || rb1_is_vertically_outside_rb2) {
		return false;
	}

	return true;
}

std::vector<RigidBody*> RigidBody::GetCollidersRigidBodyIsCollidingWith() {
	std::vector<RigidBody*> collidersBeingCollidedWith = std::vector<RigidBody*>();

	for (int w = grid_top_left.x; w <= grid_bot_right.x; w++) {
		for (int h = grid_top_left.y; h <= grid_bot_right.y; h++) {
			std::vector<RigidBody*> colliders = Singleton<GridHandler>::Get()->GetObjectsInGridLocation(w, h);

			if (colliders.size() > 1) {
				for (int c = 0; c < (int)colliders.size(); c++) {
					if (colliders[c]->only_collide_with_platforms ||
						(colliders[c]->entity_type == Constants::ENTITY_TYPE_PROJECTILE &&
						(entity_type == Constants::ENTITY_TYPE_PROJECTILE || entity_type == Constants::ENTITY_TYPE_DRONE))) {
						continue;
					}
					if (id != colliders[c]->id) {
						if ((std::find(entities_excluded_from_collision.begin(), entities_excluded_from_collision.end(), colliders[c]->entity_type) == entities_excluded_from_collision.end())) {
							if (AreTheRigidBodiesCollidingHorizontally(this, colliders[c], true) &&
								AreTheRigidBodiesCollidingVertically(this, colliders[c], true)) {
								//bool already_in_list = false;

								if (std::find(collidersBeingCollidedWith.begin(), collidersBeingCollidedWith.end(), colliders[c]) == collidersBeingCollidedWith.end()) {
									collidersBeingCollidedWith.push_back(colliders[c]);
								}
								//for (int i = 0; i < (int)collidersBeingCollidedWith.size(); i++) {
								//	if (collidersBeingCollidedWith[i]->Constants::ENTITY_TYPE == colliders[c]->Constants::ENTITY_TYPE &&
								//		collidersBeingCollidedWith[i]->id == colliders[c]->id) {
								//		already_in_list = true;
								//		break;
								//	}
								//}
								//
								//if (!already_in_list) {
								//	collidersBeingCollidedWith.push_back(colliders[c]);
								//}
							}
						}
					}
				}
			}
		}
	}

	return collidersBeingCollidedWith;
}

void RigidBody::SetGridTopLeftX(int new_value) {
	grid_top_left.x = new_value;
};
void RigidBody::SetGridTopLeftY(int new_value) {
	grid_top_left.y = new_value;
};
void RigidBody::SetGridBotRightX(int new_value) {
	grid_bot_right.x = new_value;
};
void RigidBody::SetGridBotRightY(int new_value) {
	grid_bot_right.y = new_value;
};

float RigidBody::GetDistanceBetweenTwoPoints(sf::Vector2f point_a, sf::Vector2f point_b) {
	return sqrt((point_a.x - point_b.x) * (point_a.x - point_b.x) + (point_a.y - point_b.y) * (point_a.y - point_b.y));
}

void RigidBody::ExcludeFromCollision(int ent_typ) {
	entities_excluded_from_collision.push_back(ent_typ);
}

void RigidBody::ClearEntitiesExcludedFromCollision() {
	entities_excluded_from_collision.erase(entities_excluded_from_collision.begin(), entities_excluded_from_collision.end());
}

bool RigidBody::IsFacingRight() {
	return facing_right;
}

bool RigidBody::IsInTheAir() {
	return in_the_air;
}

void RigidBody::SetInTheAir(bool in_the_air_now) {
	in_the_air = in_the_air_now;
}

void RigidBody::OnlyCollideWithPlatforms(bool only_colliding_with_platforms_now) {
	only_collide_with_platforms = only_colliding_with_platforms_now;
}

sf::Vector2f RigidBody::GetVelocity() {
	return velocity;
}

sf::Vector2f RigidBody::GetCurrentPosition() {
	return current_position; 
}

sf::Vector2f RigidBody::GetCurrentDimensions() {
	return current_dimensions;
}

std::vector<int> RigidBody::GetEntitiesExcludedFromCollision() {
	return entities_excluded_from_collision;
}

int RigidBody::GetEntityType() {
	return entity_type;
}

void RigidBody::SetCurrentPosition(sf::Vector2f new_position) {
	current_position = new_position;
}

void RigidBody::SetCurrentPosition(float x, float y) {
	current_position = sf::Vector2f(x, y);
}

void RigidBody::SetVelocity(sf::Vector2f new_velocity) {
	velocity = new_velocity;
}

void RigidBody::SetVelocity(float x, float y) {
	velocity = sf::Vector2f(x, y);
}

void RigidBody::SetEntityType(int new_entity_type) {
	entity_type = new_entity_type;
}

void RigidBody::LockFacingDirection() {
	lock_facing_direction_when_hit = true;
}

void RigidBody::UnlockFacingDirection() {
	lock_facing_direction_when_hit = false;
}

bool RigidBody::IsFacingDirectionLocked() {
	return lock_facing_direction_when_hit;
}

bool RigidBody::WasFacingRightWhenHit() {
	return facing_right_when_hit;
}

void RigidBody::SetFacingRightWhenHit(bool facing_right_when_hit) {
	facing_right = facing_right_when_hit;
}

bool RigidBody::WillOnlyCollidingWithPlatforms() {
	return only_collide_with_platforms;
}

void RigidBody::EnableCollision() {
	collision_enabled = true;
}

void RigidBody::DisableCollision() {
	collision_enabled = false;
}

void RigidBody::EnableGravity() {
	gravity_enabled = true;
}

void RigidBody::DisableGravity() {
	gravity_enabled = false;
}

int RigidBody::GetID() {
	return id;
}

int RigidBody::GetGridTopLeftX() {
	return grid_top_left.x;
}

int RigidBody::GetGridTopLeftY() {
	return grid_top_left.y;
}

int RigidBody::GetGridBotRightX() {
	return grid_bot_right.x;
}

int RigidBody::GetGridBotRightY() {
	return grid_bot_right.y;
}