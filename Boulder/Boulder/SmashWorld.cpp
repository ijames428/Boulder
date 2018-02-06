#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "SmashWorld.h"
#include "Constants.h"
#include <thread>

typedef void(*callback_function)(void); // type for conciseness

SmashWorld::SmashWorld() {
}

void SmashWorld::Init(sf::RenderWindow* window, Camera* cam) {
	render_window = window;
	camera = cam;

	//if (!shader.loadFromFile("Shaders/colorShading.vert", "Shaders/colorShading.frag"))
	//{
	//	cout << "Unable to find colorShading.vert\n";
	//}

	//if (!shader.loadFromFile("Shaders/colorShading.frag", sf::Shader::Fragment))
	//{
	//	cout << "Unable to find colorShading.frag\n";
	//}
	if (!shader.loadFromFile("Shaders/BackgroundWarp.frag", sf::Shader::Fragment))
	{
		cout << "Unable to find BackgroundWarp.frag\n";
	}

	shader.setUniform("texture", sf::Shader::CurrentTexture);

	//if (!lighting_shader.loadFromFile("Shaders/LightingShader.frag", sf::Shader::Fragment))
	//{
	//	cout << "Unable to find LightingShader.frag\n";
	//}
	//lighting_shader.setUniform("lightColor", sf::Glsl::Vec4(0.0f, 1.0f, 1.0f, 0.5f));

	StageOfTheGame = STAGE_OF_GAME_INTRO;

	player_menu_input = new InputHandler(new MenuController(0));

#ifdef _DEBUG
	string intro_cut_scene_file_relative_path = "Cutscenes/ShadowsDieTwice.avi";
	if (!intro_cutscene.openFromFile(intro_cut_scene_file_relative_path))
	{
		cout << "Unable to open file " << intro_cut_scene_file_relative_path << "\n";
	}

	intro_cutscene.fit(0, 0, camera->viewport_dimensions.x, camera->viewport_dimensions.y);
	intro_cutscene.play();
	intro_cutscene.setVolume(0);
#else
	//intro_cutscene.play();
	//intro_cutscene.setVolume(100);
#endif

	player_menu_input->EatInputsForNumberOfFrames(1);

	render_window->setActive(false);

	std::thread thread(&SmashWorld::UpdateVideo, this);

	Setup();

	thread.join();

	render_window->setActive(true); 
}

void SmashWorld::ImportSaveData() {
	string file_path = "save_data.txt";

	Json::Value save_data;
	string rawData = "";
	string line;
	ifstream myfile(file_path, ios::binary);
	
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}
	
		myfile.close();
	} else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, save_data);

	StageOfTheGame = save_data["WorldData"]["StageOfTheGame"].asString();

	PlayerOne->ApplySaveDataToObjectData(save_data);

	if (boss_one != nullptr) {
		boss_one->ApplySaveDataToObjectData(save_data);
	}

	int enemies_size = (int)enemies.size();
	for (int i = 0; i < enemies_size; i++) {
		enemies[i]->ApplySaveDataToObjectData(save_data[enemies[i]->GetName()]);
	}

	myfile.close();
}

void ExternalResumeGame() {
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
}

void ExternalExportSaveData() {
	Singleton<SmashWorld>::Get()->ExportSaveData();
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
}

void ExternalImportSaveData() {
	Singleton<SmashWorld>::Get()->ImportSaveData();
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
}

void ExternalExitGame() {
	Singleton<SmashWorld>::Get()->ExitGame();
}

void ExternalExitToMainMenu() {
	Singleton<SmashWorld>::Get()->CloseCurrentMenu();
	Singleton<SmashWorld>::Get()->ExitToMainMenu();
}

void SmashWorld::ExitGame() {
	render_window->close();
}

void SmashWorld::ExportSaveData() {
	ofstream ofs("save_data.txt", ios::binary);

	Json::Value save_data;

	save_data["WorldData"]["StageOfTheGame"] = StageOfTheGame;

	PlayerOne->ApplyObjectDataToSaveData(save_data);

	if (boss_one != nullptr) {
		boss_one->ApplyObjectDataToSaveData(save_data);
	}

	int enemies_size = (int)enemies.size();
	for (int i = 0; i < enemies_size; i++) {
		enemies[i]->ApplyObjectDataToSaveData(save_data[enemies[i]->GetName()]);
	}

	Json::StreamWriterBuilder wbuilder;
	std::string document = Json::writeString(wbuilder, save_data);

	ofs << document;

	ofs.close();

	cout << "########## Saved ##########\n";
}

void SmashWorld::PlayerDied() {
	DeadMenu->Open();
}

void SmashWorld::CloseCurrentMenu() {
	if (PauseMenu->IsOpen) {
		PauseMenu->Close();
		player_character_input->EatInputsForNumberOfFrames(1);
	} else if (DeadMenu->IsOpen) {
		DeadMenu->Close();
		player_character_input->EatInputsForNumberOfFrames(1);
	}
}

void SmashWorld::Setup() {
	exit_to_main_menu = false;
	unit_type_player_is_talking_to = "";
	boss_one_fight_started = false;

	gravity = new b2Vec2(0.0f, 30.0f);
	world = new b2World(*gravity);

	//ParseWorld("Maps\\ComboTesting");
	ParseWorld("Maps\\BossOneBugFixing0");
	ParsePlayerBestiary("Units\\PlayerBestiary.txt");
	ParseBestiaries();
	ParseDialogue("BoulderDialogue.txt");

	parallax_background_texture.loadFromFile("Images/parallax_background.jpg");
	parallax_background_sprite = sf::Sprite(parallax_background_texture);
	parallax_background_sprite.setPosition(0.0f, 0.0f);

	BuildWorld();

	timeStep = 1.0f / 60.0f;
	velocityIterations = 6;
	positionIterations = 2;

	test_drawable = new Drawable(render_window, camera);
	test_drawable->SetFlags(b2Draw::e_shapeBit);
	
	world->SetDebugDraw(test_drawable);

	world->SetContactListener(&myContactListenerInstance);

	ringbearer_font.loadFromFile("Images/RingbearerFont.ttf");

	dialogue_text = sf::Text("", ringbearer_font, 45);
	dialogue_text.setPosition(100.0f, camera->viewport_dimensions.y - 200.0f);

	CurrentDialogueLine = nullptr;
	RootDialogueLine = new DialogueLine("1", nullptr, jsonDialogueData);

	past_setup = true;

	PauseMenu = new Menu(render_window, camera->viewport_dimensions);
	PauseMenu->AddItem("Resume Game", &ExternalResumeGame);
	PauseMenu->AddItem("Save Game", &ExternalExportSaveData);
	PauseMenu->AddItem("Load Game", &ExternalImportSaveData);
	PauseMenu->AddItem("Go To Main Menu", &ExternalExitToMainMenu);
	PauseMenu->AddItem("Exit Game", &ExternalExitGame);

	DeadMenu = new Menu(render_window, camera->viewport_dimensions);
	DeadMenu->AddItem("Load Game", &ExternalImportSaveData);
	DeadMenu->AddItem("Go To Main Menu", &ExternalExitToMainMenu);
	DeadMenu->AddItem("Exit Game", &ExternalExitGame);

	//parallax_background_texture.loadFromFile("Images/parallax_background.jpg");
	//parallax_background_sprite = sf::Sprite(parallax_background_texture);
	//parallax_background_sprite.setPosition(0.0f, 0.0f);
	//parallax_background_sprite.setScale(0.4f, 0.4f);
}

void SmashWorld::UpdateVideo() {
	render_window->setActive(true);

#ifdef _DEBUG
	while (intro_cutscene.getStatus() == sfe::Playing) {
		render_window->clear();
	
		player_menu_input->Update();
	
		intro_cutscene.update();
		render_window->draw(intro_cutscene);
	
		render_window->display();
	}
#else
#endif

	render_window->setActive(false);
}

void SmashWorld::Update(sf::Int64 curr_frame, sf::Int64 frame_delta) {
	if (!render_window->hasFocus()) {
		return;
	}

	render_window->clear();

#ifdef _DEBUG
	if (intro_cutscene.getStatus() == sfe::Playing) {
		player_menu_input->Update();
	
		intro_cutscene.update();
		render_window->draw(intro_cutscene);
	} else 
#else
#endif
	if (PauseMenu->IsOpen) {
		player_menu_input->Update();
		PauseMenu->Draw(curr_frame);
	} else if (DeadMenu->IsOpen) {
		player_menu_input->Update();
		DeadMenu->Draw(curr_frame);
	} else {
		current_frame = curr_frame;

		world->Step(timeStep, velocityIterations, positionIterations);

		camera->Update(current_frame, frame_delta, PlayerOne->GetBody()->GetPosition().x, PlayerOne->GetBody()->GetPosition().y);

		if (unit_type_player_is_talking_to != "") {
			player_menu_input->Update();
		} else {
			player_character_input->Update();
		}

		parallax_background_viewport_position = sf::Vector2f(-(camera->viewport_position.x * 5.0f), -(camera->viewport_position.y * 5.0f));
		parallax_background_sprite.setPosition(parallax_background_viewport_position);

		shader.setUniform("timer", (float)curr_frame);
		shader.setUniform("x_magnitude", 0.0f);// 0.035f);
		shader.setUniform("y_magnitude", 0.0f);// 0.015f);

		//lighting_shader.setUniform("lightPosition", sf::Glsl::Vec2((PlayerOne->GetBody()->GetPosition().x - camera->viewport_position.x) * 40.0f, (PlayerOne->GetBody()->GetPosition().y - camera->viewport_position.y) * 40.0f));
		render_window->draw(parallax_background_sprite, &shader);

		world->DrawDebugData();

		PlayerOne->Update(current_frame, frame_delta);
		PlayerOne->Draw(camera->viewport_position);

		sf::Vector2f player_screen_pos = PlayerOne->GetScreenPosition();

		int doors_size = (int)doors.size();
		for (int i = 0; i < doors_size; i++) {
			doors[i]->Update(current_frame, frame_delta);
		}

		if (boss_one_fight_started) {
			boss_one->Update(current_frame, frame_delta);
			boss_one->Draw(camera->viewport_position);
		}

		int enemies_size = (int)enemies.size();
		for (int i = 0; i < enemies_size; i++) {
			if (enemies[i]->IfShouldUpdate(player_screen_pos, camera->viewport_dimensions)) {
				enemies[i]->Update(current_frame, frame_delta);
				enemies[i]->Draw(camera->viewport_position);
			}
		}

		render_window->draw(dialogue_text);
	}		

	render_window->display();
}

void SmashWorld::ScreenShake(float magnitude) {
	camera->ScreenShake(magnitude);
}

void SmashWorld::ParseWorld(string file_path) {
	string rawData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	} else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, jsonWorldData);
}

void SmashWorld::ParsePlayerBestiary(string file_path) {
	string rawData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	} else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, jsonPlayerData);
}

void SmashWorld::ParseDialogue(string file_path) {
	string rawData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	}
	else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, jsonDialogueData);
}

void SmashWorld::BuildWorld() {
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	std::string::size_type sz;

	float scalingRatio = 10.0f;

	Json::Value player_data = jsonWorldData["player"];
	Json::Value rectangles_data = jsonWorldData["rectangles"];
	Json::Value triangles_data = jsonWorldData["triangles"];
	Json::Value doors_data = jsonWorldData["doors"];
	Json::Value triggers_data = jsonWorldData["triggers"];
	Json::Value units_data = jsonWorldData["units"];

	x = std::stof(player_data["x"].asString(), &sz) / scalingRatio;
	y = std::stof(player_data["y"].asString(), &sz) / scalingRatio;
	width = std::stof(player_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
	height = std::stof(player_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

	x += width;
	y += height;

	PlayerOne = new SmashCharacter(0, jsonPlayerData, render_window, sf::Vector2f(x, y), sf::Vector2f(0.3f, 1.0f));
	PlayerOne->SetName(jsonWorldData["player"]["name"].asString());
	player_character_input = new InputHandler(PlayerOne);

	sf::Vector2f camera_position = camera->viewport_position;
	camera_position.x += (PlayerOne->GetBody()->GetPosition().x - camera->viewport_dimensions.x / 2.0f / 40.0f - camera_position.x);
	camera_position.y += (PlayerOne->GetBody()->GetPosition().y - camera->viewport_dimensions.y / 2.0f / 40.0f - camera_position.y);
	camera->viewport_position = camera_position;

	Json::Value rectangle_data;
	int rectangles_data_size = (int)rectangles_data.size();
	for (int i = 0; i < rectangles_data_size; i++) {
		rectangle_data = rectangles_data[i];

		x = std::stof(rectangle_data["x"].asString(), &sz) / scalingRatio;
		y = std::stof(rectangle_data["y"].asString(), &sz) / scalingRatio;
		width = std::stof(rectangle_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(rectangle_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		box2dRigidBodies.push_back(new Box2DRigidBody(render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
	}

	Json::Value triangle_data;
	int triangles_data_size = (int)triangles_data.size();
	for (int i = 0; i < triangles_data_size; i++) {
		triangle_data = triangles_data[i];

		std::vector<string> vects;
		vects.push_back(triangle_data["points"][0].asString());
		vects.push_back(triangle_data["points"][1].asString());
		vects.push_back(triangle_data["points"][2].asString());

		box2dRigidBodies.push_back(new Box2DRigidBody(render_window, vects));
	}

	Json::Value door_data;
	int doors_data_size = (int)doors_data.size();
	for (int i = 0; i < doors_data_size; i++) {
		door_data = doors_data[i];

		x = std::stof(door_data["x"].asString(), &sz) / scalingRatio;
		y = std::stof(door_data["y"].asString(), &sz) / scalingRatio;
		width = std::stof(door_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(door_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		doors.push_back(new Door(render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
		doors[i]->AddActivator(jsonWorldData["doors"][i]["activator"].asString());
	}

	Json::Value trigger_data;
	int triggers_data_size = (int)triggers_data.size();
	for (int i = 0; i < triggers_data_size; i++) {
		trigger_data = triggers_data[i];

		x = std::stof(trigger_data["x"].asString(), &sz) / scalingRatio;
		y = std::stof(trigger_data["y"].asString(), &sz) / scalingRatio;
		width = std::stof(trigger_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(trigger_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		triggers.push_back(new Trigger(trigger_data["name"].asString(), render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
		triggers[i]->AddActivaty(trigger_data["activity"].asString());
	}

	Json::Value unit_data;
	int units_data_size = (int)units_data.size();
	for (int i = 0; i < units_data_size; i++) {
		unit_data = units_data[i];

		x = std::stof(unit_data["LevelLocationX"].asString(), &sz) / scalingRatio;
		y = std::stof(unit_data["LevelLocationY"].asString(), &sz) / scalingRatio;
		width = std::stof(unit_data["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(unit_data["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		Json::Value thisBestiary;
		for (int j = 0; j < (int)jsonBestiariesData.size(); j++) {
			if (jsonBestiariesData[j]["BestiaryName"] == unit_data["BestiaryName"].asString()) {
				thisBestiary = jsonBestiariesData[j];
			}
		}

		//string test0 = jsonBestiariesData[0]["BestiaryName"].asString();
		//string test1 = thisBestiary["DictOfUnits"]["Gelly"]["IdleAnimations"][0]["FilePath"].asString();

		if (Contains(unit_data["UnitType"].asString(), "BossOne")) {
			boss_one = new BossOne(unit_data["InstanceOfUnitName"].asString(), unit_data["UnitType"].asString(),
				unit_data["BestiaryName"].asString(), unit_data["IsInteractable"].asBool(), thisBestiary,
				render_window, sf::Vector2f(x, y), sf::Vector2f(width, height));
			boss_one->AddActivaty(unit_data["activity"].asString());
		} else {
			enemies.push_back(new BoulderCreature(unit_data["InstanceOfUnitName"].asString(), unit_data["UnitType"].asString(),
				unit_data["BestiaryName"].asString(), unit_data["IsInteractable"].asBool(), thisBestiary,
				render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
			if (!unit_data["activity"].isNull()) {
				string test = unit_data["activity"].asString();
				enemies[(int)enemies.size() - 1]->AddActivaty(test);
			}
		}
	}

}

void SmashWorld::ParseBestiaries() {
	int bestiary_file_paths_size = (int)jsonWorldData["bestiaryFilePaths"].size();
	for (int i = 0; i < bestiary_file_paths_size; i++)
	{
		ParseBestiary(jsonWorldData["bestiaryFilePaths"][i].asString());
	}
}

void SmashWorld::ParseBestiary(string file_path) {
	size_t findResult = file_path.find("Units\\");
	string relativeFilePath = file_path.substr(findResult);

	string rawData = "";
	Json::Value jsonData = "";
	string line;
	ifstream myfile(relativeFilePath);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	} else {
		cout << "Unable to open file " << relativeFilePath << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, jsonData);

	jsonBestiariesData.push_back(jsonData);
}

void SmashWorld::SetDialogueText(string new_text) {
	dialogue_text.setString(new_text);
}

void SmashWorld::StartDialogue(string unit_type) {
	unit_type_player_is_talking_to = unit_type;
	ProgressDialogueText();
}

void SmashWorld::ProgressDialogueText() {
	if (CurrentDialogueLine == nullptr) {
		CurrentDialogueLine = RootDialogueLine;
	} else if (CurrentDialogueLine->GetNextDialogueLine(StageOfTheGame) != nullptr) {
		CurrentDialogueLine = CurrentDialogueLine->GetNextDialogueLine(StageOfTheGame);
	}

	player_menu_input->EatInputsForNumberOfFrames(1);

	dialogue_text.setString(CurrentDialogueLine->Line);
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}

void SmashWorld::ExecuteAction(string action_call) {
	std::vector<string> vstrings = split(action_call, '(');

	string call = vstrings[0];
	string argument = vstrings[1].substr(0, vstrings[1].size() - 1);

	if (call == "OpenDoor") {
		int doors_size = (int)doors.size();
		for (int i = 0; i < doors_size; i++) {
			doors[i]->TryToActivate(vstrings[1].substr(0, vstrings[1].size() - 1));
		}
	} else if (call == "ChangeStageOnDeath" || call == "ChangeStage") {
		StageOfTheGame = argument;
	} else if (call == "Aggro") {
		if (boss_one->GetName() == argument) {
			boss_one->Aggro((BoulderCreature*)PlayerOne);
			boss_one_fight_started = true;
		}
	}
}

void SmashWorld::ExitToMainMenu() {
	exit_to_main_menu = true;
}

bool SmashWorld::ShouldExitToMainMenu() {
	return exit_to_main_menu;
}

string SmashWorld::GetCurrentPointInGame() {
	return "Intro";
}

void SmashWorld::HandleLeftStickInput(float horizontal, float vertical) {
	if (PauseMenu->IsOpen || DeadMenu->IsOpen) {
		if (vertical > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			if (PauseMenu->IsOpen) {
				PauseMenu->MoveCursorDown();
			} else if (DeadMenu->IsOpen) {
				DeadMenu->MoveCursorDown();
			}
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (vertical < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			if (PauseMenu->IsOpen) {
				PauseMenu->MoveCursorUp();
			} else if (DeadMenu->IsOpen) {
				DeadMenu->MoveCursorUp();
			}
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (vertical >= -90.0f && vertical <= 90.0f) {
			can_take_another_left_stick_input_from_menu_controller = true;
		}
	}
}

void SmashWorld::HandleRightStickInput(float horizontal, float vertical) {
}

void SmashWorld::HandleButtonBPress() {
	if (PauseMenu->IsOpen) {
		PauseMenu->Close();
	} else if (DeadMenu->IsOpen) {
		DeadMenu->Close();
	} else if (unit_type_player_is_talking_to != "") {
		unit_type_player_is_talking_to = "";
		CurrentDialogueLine = nullptr;
		dialogue_text.setString("");
	}
}

void SmashWorld::HandleButtonBRelease() {
}

void SmashWorld::HandleButtonXPress() {
	if (unit_type_player_is_talking_to != "") {
		ProgressDialogueText();
	}
}

void SmashWorld::HandleButtonXRelease() {
}

void SmashWorld::HandleButtonAPress() {
#ifdef _DEBUG
	if (intro_cutscene.getStatus() == sfe::Playing) {
		intro_cutscene.stop();
	} else 
#else
#endif
	if (PauseMenu->IsOpen) {
		PauseMenu->ExecuteCurrentSelection();
	} else if (DeadMenu->IsOpen) {
		DeadMenu->ExecuteCurrentSelection();
	}
}

void SmashWorld::HandleButtonARelease() {
}

void SmashWorld::HandleButtonStartPress() {
#ifdef _DEBUG
	if (intro_cutscene.getStatus() == sfe::Playing) {
		intro_cutscene.stop();
	} else 
#else
#endif
	if (past_setup) {
		PauseMenu->Open();
	}
}

void SmashWorld::HandleButtonStartRelease() {
}

void SmashWorld::HandleButtonSelectPress() {
}

void SmashWorld::HandleButtonSelectRelease() {
}