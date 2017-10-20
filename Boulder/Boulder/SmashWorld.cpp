#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "SmashWorld.h"
#include "Constants.h"
#include "PlatformerLibrary\Drawable.h"

SmashWorld::SmashWorld() {
}

void SmashWorld::Init(sf::RenderWindow* window, Camera* cam) {
	render_window = window;
	camera = cam;

	exit_multiplayer = false;

	gravity = new b2Vec2(0.0f, 30.0f);
	world = new b2World(*gravity);

	groundBodyDef.position.Set(10.0f, 10.0f);
	groundBody = world->CreateBody(&groundBodyDef);
	groundBox.SetAsBox(10.0f, 2.0f);

	fixtureDef.shape = &groundBox;
	fixtureDef.density = 0.0f;
	fixtureDef.friction = 0.9f;
	fixtureDef.m_color = new b2Color(1.0f, 1.0f, 0.0f, 1.0f);
	fixtureDef.filter.categoryBits = Singleton<SmashWorld>::Get()->PLATFORM;

	groundBody->CreateFixture(&fixtureDef);

	//bodyDef.type = b2_dynamicBody;
	//bodyDef.position.Set(50.0f, 0.0f);
	//body = world->CreateBody(&bodyDef);

	//dynamicBox.SetAsBox(1.0f, 1.0f);
	//fixtureDef.shape = &dynamicBox;

	//fixtureDef.density = 1.0f;
	//fixtureDef.friction = 0.3f;

	//body->CreateFixture(&fixtureDef);

	timeStep = 1.0f / 60.0f;
	velocityIterations = 6;
	positionIterations = 2;

	test_drawable_rb = new Box2DRigidBody(window, sf::Vector2f(camera->viewport_dimensions.x / 2.0f, camera->viewport_dimensions.y / 2.0f), sf::Vector2f(0.3f, 1.0f), true);
	test_drawable_rb->SetFlags(b2Draw::e_shapeBit);
	
	world->SetDebugDraw(test_drawable_rb);

	PlayerOne = new SmashCharacter(0, render_window, sf::Vector2f(5.0f, 7.0f), sf::Vector2f(0.3f, 1.0f));
	//PlayerOne->SetPlayerIndex(0);
	player_one_input = new InputHandler(PlayerOne);

	PlayerTwo = new SmashCharacter(1, render_window, sf::Vector2f(7.0f, 7.0f), sf::Vector2f(0.3f, 1.0f));
	//PlayerOne->SetPlayerIndex(1);
	player_two_input = new InputHandler(PlayerTwo);

	world->SetContactListener(&myContactListenerInstance);
}

void SmashWorld::Update(sf::Int64 curr_frame, sf::Int64 frame_delta) {
	render_window->clear();

	current_frame = curr_frame;

	world->Step(timeStep, velocityIterations, positionIterations);

	world->DrawDebugData();

	player_one_input->Update();
	player_two_input->Update();

	PlayerOne->Update(current_frame, frame_delta);
	PlayerTwo->Update(current_frame, frame_delta);

	PlayerOne->Draw(camera->viewport_position);
	PlayerTwo->Draw(camera->viewport_position);

	render_window->display();
}

void SmashWorld::ExitMultiplayer() {
	exit_multiplayer = true;
}

bool SmashWorld::ShouldExitMultiplayer() {
	return exit_multiplayer;
}