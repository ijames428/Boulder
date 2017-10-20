#pragma once

#include "stdafx.h"
#include <iostream>
#include "Box2DRigidBody.h"
#include "GridHandler.h"
#include "..\GameLibrary\Singleton.h"
#include "..\Constants.h"

Box2DRigidBody::Box2DRigidBody(sf::RenderWindow *window, sf::Vector2f position, sf::Vector2f dimensions, bool subject_to_gravity, bool subject_to_collision) : Drawable::Drawable(window) {
	entity_type = Constants::ENTITY_TYPE_RIGID_BODY;
	in_the_air = true;
	facing_right = true;
}

void Box2DRigidBody::Update(sf::Int64 curr_frame, sf::Int64 delta_time) {
	current_frame = curr_frame;
}

float Box2DRigidBody::GetDistanceBetweenTwoPoints(sf::Vector2f point_a, sf::Vector2f point_b) {
	return sqrt((point_a.x - point_b.x) * (point_a.x - point_b.x) + (point_a.y - point_b.y) * (point_a.y - point_b.y));
}

bool Box2DRigidBody::IsFacingRight() {
	return facing_right;
}

sf::Vector2f Box2DRigidBody::GetCurrentPosition() {
	return current_position;
}

sf::Vector2f Box2DRigidBody::GetCurrentDimensions() {
	return current_dimensions;
}

int Box2DRigidBody::GetEntityType() {
	return entity_type;
}

void Box2DRigidBody::SetEntityType(int new_entity_type) {
	entity_type = new_entity_type;
}

void Box2DRigidBody::LockFacingDirection() {
	lock_facing_direction_when_hit = true;
}

void Box2DRigidBody::UnlockFacingDirection() {
	lock_facing_direction_when_hit = false;
}

bool Box2DRigidBody::IsFacingDirectionLocked() {
	return lock_facing_direction_when_hit;
}

bool Box2DRigidBody::WasFacingRightWhenHit() {
	return facing_right_when_hit;
}

void Box2DRigidBody::SetFacingRightWhenHit(bool facing_right_when_hit) {
	facing_right = facing_right_when_hit;
}

void Box2DRigidBody::SetFacingRight(bool new_facing_right) {
	facing_right = new_facing_right;
}

int Box2DRigidBody::GetID() {
	return id;
}

void Box2DRigidBody::SetInTheAir(bool in_the_air_now) {
	in_the_air = in_the_air_now;
}

bool Box2DRigidBody::IsInTheAir() {
	return in_the_air;
}