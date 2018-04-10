#pragma once

using namespace std;
#include "stdafx.h"
#include "MenuController.h"
#include "SmashWorld.h"

MenuController::MenuController(int player_idx) {
	player_index = player_idx;
}

void MenuController::HandleLeftStickInput(float horizontal, float vertical) {
	Singleton<SmashWorld>::Get()->HandleLeftStickInput(horizontal, vertical);
}

void MenuController::HandleRightStickInput(float horizontal, float vertical) {
	Singleton<SmashWorld>::Get()->HandleRightStickInput(horizontal, vertical);
}

void MenuController::HandleButtonBPress() {
	Singleton<SmashWorld>::Get()->HandleButtonBPress();
}

void MenuController::HandleButtonBRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonBRelease();
}

void MenuController::HandleButtonXPress() {
	Singleton<SmashWorld>::Get()->HandleButtonXPress();
}

void MenuController::HandleButtonXRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonXRelease();
}

void MenuController::HandleButtonAPress() {
	Singleton<SmashWorld>::Get()->HandleButtonAPress();
}

void MenuController::HandleButtonARelease() {
	Singleton<SmashWorld>::Get()->HandleButtonARelease();
}

void MenuController::HandleButtonStartPress() {
	Singleton<SmashWorld>::Get()->HandleButtonStartPress();
}

void MenuController::HandleButtonStartRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonStartRelease();
}

void MenuController::HandleButtonSelectPress() {
	Singleton<SmashWorld>::Get()->HandleButtonSelectPress();
}

void MenuController::HandleButtonSelectRelease() {
	Singleton<SmashWorld>::Get()->HandleButtonSelectRelease();
}

void MenuController::HandleDpadRightPress() {
	Singleton<SmashWorld>::Get()->HandleDpadRightPress();
}

void MenuController::HandleDpadRightRelease() {
	Singleton<SmashWorld>::Get()->HandleDpadRightRelease();
}

void MenuController::HandleDpadLeftPress() {
	Singleton<SmashWorld>::Get()->HandleDpadLeftPress();
}

void MenuController::HandleDpadLeftRelease() {
	Singleton<SmashWorld>::Get()->HandleDpadLeftRelease();
}

void MenuController::HandleDpadUpPress() {
	Singleton<SmashWorld>::Get()->HandleDpadUpPress();
}

void MenuController::HandleDpadUpRelease() {
	Singleton<SmashWorld>::Get()->HandleDpadUpRelease();
}

void MenuController::HandleDpadDownPress() {
	Singleton<SmashWorld>::Get()->HandleDpadDownPress();
}

void MenuController::HandleDpadDownRelease() {
	Singleton<SmashWorld>::Get()->HandleDpadDownRelease();
}