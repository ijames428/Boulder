#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "SpriteAnimation.h" 

SpriteAnimation::SpriteAnimation(sf::RenderWindow* window, std::string file_path_and_name, int frame_width, int frame_height, int total_frames, int columns_count, int rows_count, float scale, sf::Color color, bool loop) {
	render_window = window;

	current_frame = 0;
	sprite_frame_height = frame_height;
	sprite_frame_width = frame_width;
	number_of_frames = total_frames;
	number_of_columns = columns_count;
	number_of_rows = rows_count;
	sprite_scale = scale;
	facing_right = true;
	looping = loop;

	texture_rect = sf::IntRect(0, 0, sprite_frame_width, sprite_frame_height);

	//texture.loadFromFile(file_path_and_name);
	texture = Singleton<AssetManager>().Get()->GetTexture(file_path_and_name);
	//sprite = new sf::Sprite(Singleton<AssetManager>().Get()->GetTexture(file_path_and_name));
	sprite = new sf::Sprite(*texture);
	sprite->setScale(sprite_scale, sprite_scale);
	sprite->setColor(color);
}

void SpriteAnimation::Play() {
	current_frame = 0;
}

int SpriteAnimation::GetCurrentFrame() {
	return current_frame;
}

void SpriteAnimation::Draw(sf::Vector2f camera_position, sf::Vector2f parent_object_mid_position) {
	int frame_row = 0;
	int frame_column = 0;

	int next_frame = (current_frame + 1);

	if (next_frame < number_of_frames) {
		current_frame = next_frame;
	} else if (looping) {
		current_frame = next_frame % number_of_frames;
	} else {
		current_frame = number_of_frames - 1;
	}

	frame_row = current_frame / number_of_rows;
	frame_column = current_frame % number_of_rows;

	texture_rect.left = frame_column * sprite_frame_width;
	texture_rect.top = frame_row * sprite_frame_height;

	sprite->setTextureRect(texture_rect);
	sprite->setPosition(sf::Vector2f((parent_object_mid_position.x - (sprite_frame_width * sprite->getScale().x / 80.0f) - camera_position.x) * 40.0f,
		(parent_object_mid_position.y + 0.5f - (sprite_frame_height / 40.0f * sprite->getScale().y) - camera_position.y) * 40.0f));
		//-25.0f + (parent_object_mid_position.y - (sprite_frame_height * sprite->getScale().y / 2.0f) - camera_position.y) * 2.0f));
	render_window->draw(*sprite);
}

void SpriteAnimation::Flip() {
	facing_right = !facing_right;

	if (facing_right) {
		sprite->setScale(sprite_scale, sprite->getScale().y);
	}
	else {
		sprite->setScale(-sprite_scale, sprite->getScale().y);
	}
}

void SpriteAnimation::SetColor(sf::Color new_color) {
	sprite->setColor(new_color);
}

bool SpriteAnimation::IsFacingRight() {
	return facing_right;
}

int SpriteAnimation::GetNumberOfFrames() {
	return number_of_frames;
}