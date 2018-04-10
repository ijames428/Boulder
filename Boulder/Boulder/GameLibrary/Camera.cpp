#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Camera.h"
//#include "..\PlatformerLibrary\Test.h"
#include "..\GameLibrary\GameLibrary.h"

Camera::Camera(sf::Vector2f position, sf::Vector2f dimensions) {
	real_viewport_position = position;
	viewport_dimensions = dimensions;

	screen_shaking = false;
	screen_shake_start_time = 0;
	screen_shake_duration = (sf::Int64)(0.007f * 1000); // 1000 is one second in milliseconds.
}

void Camera::Update(sf::Int64 curr_frame, sf::Int64 delta_time, float player_body_position_x, float player_body_position_y) {
	current_frame = curr_frame;

	if (screen_shake_start_time + screen_shake_duration > curr_frame) {
		screen_shake_amount.x = screen_shake_magnitude * 5.0f * (rand() % 2 == 0 ? 1.0f : -1.0f);
		screen_shake_amount.y = screen_shake_magnitude * 5.0f * (rand() % 2 == 0 ? 1.0f : -1.0f);

		screen_shake_amount.x *= (rand() % 2 == 0 ? 1.0f : -1.0f);
		screen_shake_amount.y *= (rand() % 2 == 0 ? 1.0f : -1.0f);

		viewport_position_with_screen_shake = sf::Vector2f(viewport_position.x + screen_shake_amount.x / 40.0f, viewport_position.y + screen_shake_amount.y / 40.0f);

		viewport_position.x += (player_body_position_x - viewport_dimensions.x / 2.0f / 40.0f - viewport_position_with_screen_shake.x) * lerp * delta_time;
		viewport_position.y += (player_body_position_y - viewport_dimensions.y / 2.0f / 40.0f - viewport_position_with_screen_shake.y) * lerp * delta_time;
	} else {
		viewport_position.x += (player_body_position_x - viewport_dimensions.x / 2.0f / 40.0f - viewport_position.x) * lerp * delta_time;
		viewport_position.y += (player_body_position_y - viewport_dimensions.y / 2.0f / 40.0f - viewport_position.y) * lerp * delta_time;
	}

	//if (screen_shake_start_time + screen_shake_duration * 2 < curr_frame) {
	//	screen_shake_magnitude = 0.0f;
	//}
}

void Camera::ScreenShake(float magnitude) {
	screen_shake_start_time = current_frame;
	screen_shake_magnitude = magnitude;
}