#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "World.h"
#include "Constants.h"

World::World() {
}

void World::Init(sf::RenderWindow* window, Camera* cam, PlayerCharacter* character) {
	Grid = std::vector<std::vector<std::vector<RigidBody*>>>(grid_width, std::vector<std::vector<RigidBody*>>(grid_height, std::vector<RigidBody*>()));
	render_window = window;
	camera = cam;

	if (!ringbearer_font.loadFromFile("Images/RingbearerFont.ttf"))
		return;

	loading_text = sf::Text("Loading.", ringbearer_font, 60);
	loading_text.setPosition(camera->viewport_dimensions.x / 2.0f - 150.0f, camera->viewport_dimensions.y / 2.0f - 100.0f);

	UpdateLoadingText("L");

	main_character = character;
	paused = false;
	game_over_screen_sprite_transparency = 0;
	end_game_fade_transparency = 0;
	player_is_in_combat = false;
	fighting_boss = false;
	combat_music_range = 400.0f;
	current_frame = 0;
	screen_shaking = false;
	screen_shake_start_time = 0;
	screen_shake_duration = (sf::Int64)(0.01f * 1000); // 1000 is one second in milliseconds.
	time_of_death = 0;
	time_before_continue_screen_appears = 3000;

	BuildReleaseLevel();
	//BuildTestLevel();
	//BuildDevLevel();

	hit_point_texture.loadFromFile("Images/HitPoint.png");

	for (int i = 0; i < main_character->hit_points; i++) {
		players_hit_point_sprites.push_back(sf::Sprite(hit_point_texture));
		players_hit_point_sprites[i].setPosition(sf::Vector2f(10.0f + 25.0f * (float)i, 10.0f));
	}

	blank_screen_texture.loadFromFile("Images/StartMenuBackground.png");
	blank_screen_sprite = sf::Sprite(blank_screen_texture);

	UpdateLoadingText("Load");

	game_over_texture.loadFromFile("Images/GameOverScreen.png");
	game_over_sprite = sf::Sprite(game_over_texture);

	UpdateLoadingText("Loadi");

	level_art_texture.loadFromFile("Images/ReleaseLevelArt.png");
	level_art_sprite = sf::Sprite(level_art_texture);

	UpdateLoadingText("Loadin");

	foreground_behind_layer_texture.loadFromFile("Images/ReleaseLevelArt_rough_overlay.png");
	foreground_behind_layer_sprite = sf::Sprite(foreground_behind_layer_texture);

	UpdateLoadingText("Loading");

	parallax_background_texture.loadFromFile("Images/ReleaseLevelArt_Backdrop.png");
	parallax_background_sprite = sf::Sprite(parallax_background_texture);
	parallax_background_sprite.setPosition(0.0f, 0.0f);
	parallax_background_sprite.setScale(0.4f, 0.4f);

	UpdateLoadingText("Loading.");

	lives_counter_text = sf::Text();
	lives_counter_text.setFont(ringbearer_font);
	lives_counter_text.setString("Lives: " + std::to_string(current_number_of_lives));
	lives_counter_text.setPosition(10.0f, 40.0f);

	paused_text = sf::Text();
	paused_text.setFont(ringbearer_font);
	paused_text.setCharacterSize(90);
	paused_text.setString("Paused");
	paused_text.setPosition(camera->viewport_dimensions.x / 2.0f - 150.0f, camera->viewport_dimensions.y / 2.0f - 100.0f);

	continue_text = sf::Text();
	continue_text.setFont(ringbearer_font);
	continue_text.setString("You have " + std::to_string(current_number_of_lives) + " lives left.\nPress Start to continue.");
	continue_text.setPosition(camera->viewport_dimensions.x / 2.0f - 150.0f, camera->viewport_dimensions.y / 2.0f - 100.0f);

	UpdateLoadingText("Loading..");

	if (!buffer_stalagtite_landing.loadFromFile("Sound/stalagtite_landing.wav")) {
		throw exception("Sound file not found");
	}
	else {
		sound_stalagtite_landing.setBuffer(buffer_stalagtite_landing);
		sound_stalagtite_landing.setVolume(50 * (Singleton<Settings>().Get()->effects_volume / 100.0f));
		sound_stalagtite_landing.setLoop(false);
	}

	UpdateLoadingText("Loading...");

	if (!door_opening_buffer.loadFromFile("Sound/door_opening.wav")) {
		throw exception("Sound file not found");
	} else {
		door_opening_volume = 0.0f;
		door_opening_sound.setBuffer(door_opening_buffer);
		door_opening_sound.setVolume(door_opening_volume * (Singleton<Settings>().Get()->effects_volume / 100.0f));
		door_opening_sound.setLoop(true);
	}
}

void World::Update(sf::Int64 curr_time, sf::Int64 frame_delta) {
	render_window->clear();

	if (!paused)
	{
		current_frame = curr_time;
		player_is_in_combat = false;

		if (frame_delta > 250) {
			frame_delta = 50;
		}

		if (stalagtite_hit_points <= 0) {
			if (stalagtite->GetCurrentPosition().y < 1500.0f) {
				stalagtite->EnableGravity();
			}
			else if (stalagtite->GetVelocity().y != 0.0f) {
				stalagtite->DisableGravity();
				stalagtite->SetCurrentPosition(stalagtite->GetCurrentPosition().x, 1501.0f);
				stalagtite->SetVelocity(0.0f, 0.0f);
				ScreenShake(5.0f);
				sound_stalagtite_landing.play();
			}
		}

		screen_shake_amount.x = screen_shake_magnitude * 5.0f * (rand() % 2 == 0 ? 1.0f : -1.0f);
		screen_shake_amount.y = screen_shake_magnitude * 5.0f * (rand() % 2 == 0 ? 1.0f : -1.0f);

		if (screen_shake_start_time + screen_shake_duration < current_frame) {
			screen_shake_amount.x *= (rand() % 2 == 0 ? 1.0f : -1.0f);
			screen_shake_amount.y *= (rand() % 2 == 0 ? 1.0f : -1.0f);
		}

		if (screen_shake_start_time + screen_shake_duration * 2 < current_frame) {
			screen_shake_magnitude = 0.0f;
		}

		float lerp = 0.01f;
		camera_position = camera->viewport_position;
		camera_position.x += (main_character->GetCurrentPosition().x - camera->viewport_dimensions.x / 2.0f - camera_position.x) * lerp * frame_delta;
		camera_position.y += (main_character->GetCurrentPosition().y - camera->viewport_dimensions.y / 2.0f - camera_position.y) * lerp * frame_delta;
		camera->viewport_position.x = camera_position.x;
		camera->viewport_position.y = camera_position.y;

		viewport_position_with_screen_shake = sf::Vector2f(camera->viewport_position.x + screen_shake_amount.x, camera->viewport_position.y + screen_shake_amount.y);

		parallax_background_viewport_position = sf::Vector2f(-(viewport_position_with_screen_shake.x / 10.0f), -(viewport_position_with_screen_shake.y / 10.0f));
		parallax_background_sprite.setPosition(parallax_background_viewport_position);
		render_window->draw(parallax_background_sprite);

		foreground_behind_layer_sprite.setPosition(sf::Vector2f(-viewport_position_with_screen_shake.x, -viewport_position_with_screen_shake.y));
		render_window->draw(foreground_behind_layer_sprite);

		main_character->Draw(viewport_position_with_screen_shake);
		main_character->Update(current_frame, frame_delta);

		if (IsObjectInUpdateRange((RigidBody*)end_of_the_game_trigger)) {
			((RigidBody*)end_of_the_game_trigger)->Update(current_frame, frame_delta);
			end_of_the_game_trigger->UpdateEndOfTheGame();
			end_of_the_game_trigger->Draw(viewport_position_with_screen_shake);
		}
		for (int i = 0; i < (int)checkpoints.size(); i++) {
			if (IsObjectInUpdateRange((RigidBody*)checkpoints[i])) {
				((RigidBody*)checkpoints[i])->Update(current_frame, frame_delta);
				checkpoints[i]->UpdateCheckPoint();
				checkpoints[i]->Draw(viewport_position_with_screen_shake);
			}
		}

		if (IsObjectInUpdateRange((RigidBody*)end_of_game_door)) {
			end_of_game_door->Update(current_frame, frame_delta);
			end_of_game_door->Draw(viewport_position_with_screen_shake);
		}
		if (IsObjectInUpdateRange((RigidBody*)stalagtite)) {
			stalagtite->Update(current_frame, frame_delta);
			stalagtite->Draw(viewport_position_with_screen_shake);
		}
		for (int i = 0; i < (int)platforms.size(); i++) {
			if (IsObjectInUpdateRange((RigidBody*)platforms[i])) {
				platforms[i]->Update(current_frame, frame_delta);
				platforms[i]->Draw(viewport_position_with_screen_shake);
			}
		}

		level_art_sprite.setPosition(sf::Vector2f(-viewport_position_with_screen_shake.x, -viewport_position_with_screen_shake.y));
		render_window->draw(level_art_sprite);

		for (int i = 0; i < main_character->hit_points; i++) {
			render_window->draw(players_hit_point_sprites[i]);
		}

		render_window->draw(lives_counter_text);

		if (player_beat_the_game) {
			if (end_game_fade_transparency >= 255) {
				end_game_fade_transparency = 255;
			}
			else {
				end_game_fade_transparency += 5;
			}

			//main_character->SetVelocity(3.0f, main_character->GetVelocity().y);

			blank_screen_sprite.setColor(sf::Color(255, 255, 255, end_game_fade_transparency));
			blank_screen_sprite.setScale(2.0f, 2.0f);
			//blank_screen_sprite.setPosition(0.0f, 0.0f);
			render_window->draw(blank_screen_sprite);

			if (main_character->GetCurrentPosition().x > 4075.0f) {
				go_to_credits = true;
			}
		}
		else if (CanContinue()) {
			if (game_over_screen_sprite_transparency >= 255) {
				game_over_screen_sprite_transparency = 255;
			}
			else {
				game_over_screen_sprite_transparency += 1;
			}

			blank_screen_sprite.setScale(2.0f, 2.0f);

			if (current_number_of_lives > 0) {
				blank_screen_sprite.setColor(sf::Color(255, 255, 255, game_over_screen_sprite_transparency));
				render_window->draw(blank_screen_sprite);

				continue_text.setFillColor(sf::Color(255, 255, 255, game_over_screen_sprite_transparency));
				continue_text.setOutlineColor(sf::Color(255, 255, 255, game_over_screen_sprite_transparency));
				render_window->draw(continue_text);
			}
			else {
				game_over_sprite.setColor(sf::Color(255, 255, 255, game_over_screen_sprite_transparency));

				render_window->draw(game_over_sprite);
			}
		}
	}
	else {
		blank_screen_sprite.setColor(sf::Color(255, 255, 255, 255));
		render_window->draw(blank_screen_sprite);
		render_window->draw(paused_text);
		if (door_opening_sound.getStatus() == sf::Sound::Status::Playing) {
			door_opening_sound.pause();
		}
	}

	render_window->display();
}

bool World::IsObjectInUpdateRange(RigidBody* rb) {
	if (RigidBody::GetDistanceBetweenTwoPoints(sf::Vector2f(main_character->GetCurrentPosition().x + main_character->GetCurrentDimensions().x / 2.0f, main_character->GetCurrentPosition().y + main_character->GetCurrentDimensions().y / 2.0f), sf::Vector2f(rb->GetCurrentPosition().x, rb->GetCurrentPosition().y)) < camera->viewport_dimensions.x + 50.0f) {
		return true;
	}

	if (RigidBody::GetDistanceBetweenTwoPoints(sf::Vector2f(main_character->GetCurrentPosition().x + main_character->GetCurrentDimensions().x / 2.0f, main_character->GetCurrentPosition().y + main_character->GetCurrentDimensions().y / 2.0f), sf::Vector2f(rb->GetCurrentPosition().x + rb->GetCurrentDimensions().x, rb->GetCurrentPosition().y)) < camera->viewport_dimensions.x + 50.0f) {
		return true;
	}

	if (RigidBody::GetDistanceBetweenTwoPoints(sf::Vector2f(main_character->GetCurrentPosition().x + main_character->GetCurrentDimensions().x / 2.0f, main_character->GetCurrentPosition().y + main_character->GetCurrentDimensions().y / 2.0f), sf::Vector2f(rb->GetCurrentPosition().x, rb->GetCurrentPosition().y + rb->GetCurrentDimensions().y)) < camera->viewport_dimensions.x + 50.0f) {
		return true;
	}

	if (RigidBody::GetDistanceBetweenTwoPoints(sf::Vector2f(main_character->GetCurrentPosition().x + main_character->GetCurrentDimensions().x / 2.0f, main_character->GetCurrentPosition().y + main_character->GetCurrentDimensions().y / 2.0f), sf::Vector2f(rb->GetCurrentPosition().x + rb->GetCurrentDimensions().x, rb->GetCurrentPosition().y + rb->GetCurrentDimensions().y)) < camera->viewport_dimensions.x + 50.0f) {
		return true;
	}

	return false;
}

void World::UpdateLoadingText(sf::String loading_string) {
	render_window->clear();
	loading_text.setString(loading_string);
	render_window->draw(loading_text);
	render_window->display();
}

void World::ScreenShake(float magnitude) {
	screen_shake_start_time = current_frame;
	screen_shake_magnitude = magnitude;
}

void World::SetCurrentCheckPoint(Checkpoint* cp) {
	current_checkpoint = cp;
}

void World::EndTheGame() {
	if (door_opening_sound.getStatus() == sf::Sound::Status::Playing) {
		door_opening_sound.stop();
	}
	player_beat_the_game = true;
}

bool World::DidThePlayerBeatTheGame() {
	return player_beat_the_game;
}

bool World::ShouldGoToCredits() {
	return go_to_credits;
}

bool World::IsPlayerInCombat() {
	return player_is_in_combat;
}

void World::PlayerDied() {
	game_over_screen_sprite_transparency = 0;
	time_of_death = current_frame;
}

bool World::CanContinue() {
	return time_of_death + time_before_continue_screen_appears < current_frame && main_character->hit_points <= 0;
}

void World::StartNewGame() {
	player_beat_the_game = false;
	go_to_credits = false;
	current_number_of_lives = starting_number_of_lives;
}

bool World::IsNewGame() {
	return current_number_of_lives == starting_number_of_lives;
}

void World::HitStalagtite() {
	stalagtite_hit_points--;
}

void World::BuildReleaseLevel() {
	if (IsNewGame()) {
		starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(100.0f, 150.0f), sf::Vector2f(40.0f, 10.0f), false);//starting position	
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(2288.0f, 1200.0f), sf::Vector2f(40.0f, 264.0f), false);
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(1440.0f, 1976.0f), sf::Vector2f(40.0f, 120.0f), false);
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(1016.0f, 3592.0f), sf::Vector2f(40.0f, 192.0f), false);
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(1728.0f, 4752.0f), sf::Vector2f(40.0f, 192.0f), false);
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(3192.0f, 4000.0f), sf::Vector2f(40.0f, 120.0f), false);
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(3480.0f, 2456.0f), sf::Vector2f(40.0f, 152.0f), false);
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(3216.0f, 104.0f), sf::Vector2f(40.0f, 192.0f), false);
		//starting_checkpoint = new Checkpoint(render_window, sf::Vector2f(3056.0f, 1272.0f), sf::Vector2f(40.0f, 176.0f), false);
		current_checkpoint = starting_checkpoint;

		stalagtite = new Platform(render_window, sf::Vector2f(2427.0f, 569.0f), sf::Vector2f(420.0f, 236.0f));
		stalagtite->SetEntityType(Constants::ENTITY_TYPE_STALAGTITE);

		//main_character->SetVelocity(5.0f, -5.0f);
	}
	else {
		stalagtite = new Platform(render_window, sf::Vector2f(2427.0f, stalagtite->GetCurrentPosition().y), sf::Vector2f(420.0f, 236.0f));
		stalagtite->SetEntityType(Constants::ENTITY_TYPE_STALAGTITE);

		//main_character->SetVelocity(0.0f, 0.0f);
	}

	//main_character->SetCurrentPosition(((RigidBody*)current_checkpoint)->GetCurrentPosition().x, ((RigidBody*)current_checkpoint)->GetCurrentPosition().y + ((RigidBody*)current_checkpoint)->GetCurrentDimensions().y - main_character->GetCurrentDimensions().y);
	main_character->hit_points = main_character->max_hit_points;

	platforms.erase(platforms.begin(), platforms.end());
	platforms.push_back(new Platform(render_window, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(65.0f, 5000.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(5000.0f, 37.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(0.0f, 4966.0f), sf::Vector2f(5000.0f, 34.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4940.0f, 0.0f), sf::Vector2f(60.0f, 5000.0f)));

	platforms.push_back(new Platform(render_window, sf::Vector2f(0.0f, 246.0f), sf::Vector2f(356.0f, 1701.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(356.0f, 478.0f), sf::Vector2f(241.0f, 1469.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(597.0f, 740.0f), sf::Vector2f(657.0f, 1207.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1254.0f, 940.0f), sf::Vector2f(163.0f, 1008.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1417.0f, 1212.0f), sf::Vector2f(311.0f, 737.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1728.0f, 1484.0f), sf::Vector2f(302.0f, 464.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2030.0f, 1484.0f), sf::Vector2f(412.0f, 277.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2833.0f, 1482.0f), sf::Vector2f(362.0f, 242.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2504.0f, 2120.0f), sf::Vector2f(208.0f, 65.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1969.0f, 2119.0f), sf::Vector2f(292.0f, 50.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1406.0f, 2119.0f), sf::Vector2f(691.0f, 204.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1184.0f, 2250.0f), sf::Vector2f(798.0f, 309.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(496.0f, 2434.0f), sf::Vector2f(704.0f, 125.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1066.0f, 2554.0f), sf::Vector2f(124.0f, 294.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(0.0f, 1941.0f), sf::Vector2f(282.0f, 1625.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(273.0f, 2874.0f), sf::Vector2f(93.0f, 482.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(366.0f, 3095.0f), sf::Vector2f(106.0f, 265.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(0.0f, 3341.0f), sf::Vector2f(1555.0f, 223.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1552.0f, 2866.0f), sf::Vector2f(116.0f, 698.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(689.0f, 3224.0f), sf::Vector2f(45.0f, 118.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(732.0f, 3119.0f), sf::Vector2f(53.0f, 231.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(784.0f, 2979.0f), sf::Vector2f(64.0f, 370.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1980.0f, 2514.0f), sf::Vector2f(1061.0f, 1467.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2744.0f, 2380.0f), sf::Vector2f(296.0f, 1600.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2828.0f, 2257.0f), sf::Vector2f(110.0f, 145.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2915.0f, 2144.0f), sf::Vector2f(80.0f, 156.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2983.0f, 1722.0f), sf::Vector2f(215.0f, 207.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1364.0f, 2987.0f), sf::Vector2f(117.0f, 70.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1173.0f, 3127.0f), sf::Vector2f(114.0f, 61.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1369.0f, 3223.0f), sf::Vector2f(46.0f, 28.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1938.0f, 2920.0f), sf::Vector2f(43.0f, 889.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1868.0f, 3254.0f), sf::Vector2f(77.0f, 556.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(978.0f, 3793.0f), sf::Vector2f(925.0f, 151.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1120.0f, 3942.0f), sf::Vector2f(94.0f, 183.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1214.0f, 3921.0f), sf::Vector2f(163.0f, 444.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1322.0f, 3970.0f), sf::Vector2f(657.0f, 731.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(46.0f, 3537.0f), sf::Vector2f(114.0f, 594.0f)));//
	platforms.push_back(new Platform(render_window, sf::Vector2f(176.0f, 4805.0f), sf::Vector2f(139.0f, 163.0f)));//
	platforms.push_back(new Platform(render_window, sf::Vector2f(58.0f, 4566.0f), sf::Vector2f(119.0f, 408.0f)));//
	platforms.push_back(new Platform(render_window, sf::Vector2f(2394.0f, 4265.0f), sf::Vector2f(573.0f, 128.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2492.0f, 4383.0f), sf::Vector2f(387.0f, 150.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2566.0f, 4531.0f), sf::Vector2f(193.0f, 262.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2158.0f, 4748.0f), sf::Vector2f(1018.0f, 241.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3174.0f, 4134.0f), sf::Vector2f(489.0f, 869.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2103.0f, 4855.0f), sf::Vector2f(59.0f, 114.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2265.0f, 4623.0f), sf::Vector2f(43.0f, 141.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2061.0f, 4507.0f), sf::Vector2f(143.0f, 37.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(1970.0f, 4373.0f), sf::Vector2f(73.0f, 43.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2162.0f, 4267.0f), sf::Vector2f(120.0f, 46.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2893.0f, 4620.0f), sf::Vector2f(95.0f, 36.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3022.0f, 4497.0f), sf::Vector2f(121.0f, 34.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3079.0f, 4134.0f), sf::Vector2f(116.0f, 32.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3108.0f, 4361.0f), sf::Vector2f(74.0f, 143.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3174.0f, 4134.0f), sf::Vector2f(489.0f, 859.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3658.0f, 4192.0f), sf::Vector2f(430.0f, 804.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4079.0f, 4150.0f), sf::Vector2f(740.0f, 837.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4363.0f, 4040.0f), sf::Vector2f(633.0f, 139.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4417.0f, 3951.0f), sf::Vector2f(579.0f, 99.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4473.0f, 3847.0f), sf::Vector2f(525.0f, 114.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4519.0f, 3744.0f), sf::Vector2f(480.0f, 111.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4555.0f, 3604.0f), sf::Vector2f(441.0f, 153.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4584.0f, 3286.0f), sf::Vector2f(410.0f, 325.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4547.0f, 3361.0f), sf::Vector2f(45.0f, 42.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4268.0f, 3466.0f), sf::Vector2f(196.0f, 97.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4648.0f, 2848.0f), sf::Vector2f(348.0f, 452.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4471.0f, 3160.0f), sf::Vector2f(57.0f, 44.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4266.0f, 3060.0f), sf::Vector2f(99.0f, 66.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4345.0f, 2671.0f), sf::Vector2f(649.0f, 185.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3930.0f, 2995.0f), sf::Vector2f(154.0f, 91.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3622.0f, 3088.0f), sf::Vector2f(138.0f, 69.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3342.0f, 2984.0f), sf::Vector2f(133.0f, 70.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3250.0f, 2868.0f), sf::Vector2f(123.0f, 52.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3200.0f, 2813.0f), sf::Vector2f(110.0f, 72.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3020.0f, 3489.0f), sf::Vector2f(153.0f, 491.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3038.0f, 2610.0f), sf::Vector2f(137.0f, 65.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3146.0f, 2665.0f), sf::Vector2f(29.0f, 82.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3438.0f, 2677.0f), sf::Vector2f(23.0f, 63.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3409.0f, 2721.0f), sf::Vector2f(48.0f, 19.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3438.0f, 2618.0f), sf::Vector2f(1557.0f, 65.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2984.0f, 1920.0f), sf::Vector2f(1587.0f, 499.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4564.0f, 2304.0f), sf::Vector2f(95.0f, 114.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4842.0f, 2295.0f), sf::Vector2f(138.0f, 126.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4334.0f, 335.0f), sf::Vector2f(237.0f, 1590.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3908.0f, 336.0f), sf::Vector2f(482.0f, 979.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3643.0f, 267.0f), sf::Vector2f(456.0f, 94.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(3204.0f, 336.0f), sf::Vector2f(703.0f, 577.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(2871.0f, 681.0f), sf::Vector2f(343.0f, 233.0f)));//
	platforms.push_back(new Platform(render_window, sf::Vector2f(2033.0f, 25.0f), sf::Vector2f(944.0f, 545.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4817.0f, 2405.0f), sf::Vector2f(28.0f, 107.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4790.0f, 2491.0f), sf::Vector2f(53.0f, 21.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4558.0f, 2164.0f), sf::Vector2f(127.0f, 31.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4815.0f, 2033.0f), sf::Vector2f(151.0f, 34.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4551.0f, 1897.0f), sf::Vector2f(136.0f, 34.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4815.0f, 1766.0f), sf::Vector2f(146.0f, 33.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4550.0f, 1630.0f), sf::Vector2f(138.0f, 32.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4817.0f, 1499.0f), sf::Vector2f(135.0f, 33.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4560.0f, 1362.0f), sf::Vector2f(129.0f, 33.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4817.0f, 1231.0f), sf::Vector2f(132.0f, 34.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4565.0f, 1095.0f), sf::Vector2f(125.0f, 36.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4816.0f, 964.0f), sf::Vector2f(131.0f, 34.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4554.0f, 828.0f), sf::Vector2f(135.0f, 32.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4816.0f, 697.0f), sf::Vector2f(134.0f, 33.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4561.0f, 560.0f), sf::Vector2f(129.0f, 34.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4817.0f, 429.0f), sf::Vector2f(128.0f, 35.0f)));
	platforms.push_back(new Platform(render_window, sf::Vector2f(4559.0f, 335.0f), sf::Vector2f(131.0f, 35.0f)));

	UpdateLoadingText("Lo");

	checkpoints.erase(checkpoints.begin(), checkpoints.end());
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(2288.0f, 1200.0f), sf::Vector2f(40.0f, 264.0f), false));
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(1440.0f, 1976.0f), sf::Vector2f(40.0f, 120.0f), false));
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(1016.0f, 3592.0f), sf::Vector2f(40.0f, 192.0f), false));
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(1728.0f, 4752.0f), sf::Vector2f(40.0f, 192.0f), false));
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(3192.0f, 4000.0f), sf::Vector2f(40.0f, 120.0f), false));
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(3480.0f, 2456.0f), sf::Vector2f(40.0f, 152.0f), false));
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(3216.0f, 104.0f), sf::Vector2f(40.0f, 192.0f), false));
	checkpoints.push_back(new Checkpoint(render_window, sf::Vector2f(3056.0f, 1272.0f), sf::Vector2f(40.0f, 176.0f), false));

	end_of_the_game_trigger = new EndOfTheGame(render_window, sf::Vector2f(3950.0f, 1408.0f), sf::Vector2f(200.0f, 488.0f), false);
	boss_health_trigger = new RigidBody(sf::Vector2f(3224.0f, 1824.0f), sf::Vector2f(672.0f, 88.0f), false, false);
	boss_health_trigger->SetEntityType(Constants::ENTITY_TYPE_BOSS_TRIGGER);
	end_of_game_door = new Platform(render_window, sf::Vector2f(3912.0f, 1304.0f), sf::Vector2f(112.0f, 640.0f));
}