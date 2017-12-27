#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "SmashWorld.h"
#include "Constants.h"

SmashWorld::SmashWorld() {
}

void SmashWorld::Init(sf::RenderWindow* window, Camera* cam) {
	render_window = window;
	camera = cam;

	exit_multiplayer = false;

	gravity = new b2Vec2(0.0f, 30.0f);
	world = new b2World(*gravity);

	ParseWorld("C:\\Users\\Ian\\Documents\\GitHub\\Boulder\\Boulder\\Boulder\\Maps\\testmap00");
	BuildWorld();

	timeStep = 1.0f / 60.0f;
	velocityIterations = 6;
	positionIterations = 2;

	test_drawable = new Drawable(window, camera);
	test_drawable->SetFlags(b2Draw::e_shapeBit);
	
	world->SetDebugDraw(test_drawable);

	world->SetContactListener(&myContactListenerInstance);
}

void SmashWorld::ParseWorld(string file_path) {
	rawWorldData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawWorldData += line;
		}

		myfile.close();
	} else {
		cout << "Unable to open file";
	}

	Json::Reader reader;
	reader.parse(rawWorldData, jsonWorldData);
}

void SmashWorld::BuildWorld() {
	float x = 0.0f;
	float y = 0.0f;
	float width = 0.0f;
	float height = 0.0f;
	std::string::size_type sz;

	float scalingRatio = 10.0f;

	x = std::stof(jsonWorldData["player"]["x"].asString(), &sz) / scalingRatio;
	y = std::stof(jsonWorldData["player"]["y"].asString(), &sz) / scalingRatio;
	width = std::stof(jsonWorldData["player"]["width"].asString(), &sz) / (scalingRatio * 2.0f);
	height = std::stof(jsonWorldData["player"]["height"].asString(), &sz) / (scalingRatio * 2.0f);

	x += width;
	y += height;

	PlayerOne = new SmashCharacter(0, render_window, sf::Vector2f(x, y), sf::Vector2f(0.3f, 1.0f));
	PlayerOne->SetName(jsonWorldData["player"]["name"].asString());
	player_one_input = new InputHandler(PlayerOne);

	sf::Vector2f camera_position = camera->viewport_position;
	camera_position.x += (PlayerOne->GetBody()->GetPosition().x - camera->viewport_dimensions.x / 2.0f / 40.0f - camera_position.x);
	camera_position.y += (PlayerOne->GetBody()->GetPosition().y - camera->viewport_dimensions.y / 2.0f / 40.0f - camera_position.y);
	camera->viewport_position = camera_position;

	for (int i = 0; i < (int)jsonWorldData["rectangles"].size(); i++) {
		x = std::stof(jsonWorldData["rectangles"][i]["x"].asString(), &sz) / scalingRatio;
		y = std::stof(jsonWorldData["rectangles"][i]["y"].asString(), &sz) / scalingRatio;
		width = std::stof(jsonWorldData["rectangles"][i]["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(jsonWorldData["rectangles"][i]["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		box2dRigidBodies.push_back(new Box2DRigidBody(render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
	}

	for (int i = 0; i < (int)jsonWorldData["triangles"].size(); i++) {
		std::vector<string> vects;
		vects.push_back(jsonWorldData["triangles"][i]["points"][0].asString());
		vects.push_back(jsonWorldData["triangles"][i]["points"][1].asString());
		vects.push_back(jsonWorldData["triangles"][i]["points"][2].asString());

		box2dRigidBodies.push_back(new Box2DRigidBody(render_window, vects));
	}

	for (int i = 0; i < (int)jsonWorldData["doors"].size(); i++) {
		x = std::stof(jsonWorldData["doors"][i]["x"].asString(), &sz) / scalingRatio;
		y = std::stof(jsonWorldData["doors"][i]["y"].asString(), &sz) / scalingRatio;
		width = std::stof(jsonWorldData["doors"][i]["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(jsonWorldData["doors"][i]["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		doors.push_back(new Door(render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
		doors[i]->AddActivator(jsonWorldData["doors"][i]["activator"].asString());
	}

	ParseBestiaries();

	for (int i = 0; i < (int)jsonWorldData["units"].size(); i++) {
		x = std::stof(jsonWorldData["units"][i]["LevelLocationX"].asString(), &sz) / scalingRatio;
		y = std::stof(jsonWorldData["units"][i]["LevelLocationY"].asString(), &sz) / scalingRatio;
		width = std::stof(jsonWorldData["units"][i]["width"].asString(), &sz) / (scalingRatio * 2.0f);
		height = std::stof(jsonWorldData["units"][i]["height"].asString(), &sz) / (scalingRatio * 2.0f);

		x += width;
		y += height;

		Json::Value thisBestiary;
		for (int j = 0; j < (int)jsonBestiariesData.size(); j++) {
			if (jsonBestiariesData[j]["BestiaryName"] == jsonWorldData["units"][i]["BestiaryName"].asString()) {
				thisBestiary = jsonBestiariesData[j];
			}
		}

		//string test0 = jsonBestiariesData[0]["BestiaryName"].asString();
		//string test1 = thisBestiary["DictOfUnits"]["Gelly"]["IdleAnimations"][0]["FilePath"].asString();

		enemies.push_back(new BoulderCreature(jsonWorldData["units"][i]["InstanceOfUnitName"].asString(), jsonWorldData["units"][i]["UnitType"].asString(), 
											  jsonWorldData["units"][i]["BestiaryName"].asString(), thisBestiary,
											  render_window, sf::Vector2f(x, y), sf::Vector2f(width, height)));
	}

}

void SmashWorld::ParseBestiaries() {
	for (int i = 0; i < (int)jsonWorldData["bestiaryFilePaths"].size(); i++)
	{
		ParseBestiary(jsonWorldData["bestiaryFilePaths"][i].asString());
	}
}

void SmashWorld::ParseBestiary(string file_path) {
	string rawData = "";
	Json::Value jsonData = "";
	string line;
	ifstream myfile(file_path);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	}
	else {
		cout << "Unable to open file";
	}

	Json::Reader reader;
	reader.parse(rawData, jsonData);

	jsonBestiariesData.push_back(jsonData);
}

void SmashWorld::Update(sf::Int64 curr_frame, sf::Int64 frame_delta) {
	render_window->clear();

	current_frame = curr_frame;

	world->Step(timeStep, velocityIterations, positionIterations);

	float lerp = 0.01f;
	sf::Vector2f camera_position = camera->viewport_position;

	camera_position.x += (PlayerOne->GetBody()->GetPosition().x - camera->viewport_dimensions.x / 2.0f / 40.0f - camera_position.x) * lerp * frame_delta;
	camera_position.y += (PlayerOne->GetBody()->GetPosition().y - camera->viewport_dimensions.y / 2.0f / 40.0f - camera_position.y) * lerp * frame_delta;

	camera->viewport_position = camera_position;

	world->DrawDebugData();

	player_one_input->Update();

	PlayerOne->Update(current_frame, frame_delta);
	PlayerOne->Draw(camera->viewport_position);

	for (int i = 0; i < (int)doors.size(); i++) {
		doors[i]->Update(current_frame, frame_delta);
	}

	for (int i = 0; i < (int)enemies.size(); i++) {
		enemies[i]->Update(current_frame, frame_delta);
		enemies[i]->Draw(camera->viewport_position);
	}

	render_window->display();
}

void SmashWorld::ExitMultiplayer() {
	exit_multiplayer = true;
}

bool SmashWorld::ShouldExitMultiplayer() {
	return exit_multiplayer;
}