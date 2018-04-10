#pragma once

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

using namespace std;
#include "ControllableCharacter.h"

class MenuController : public ControllableCharacter {
private:
	int player_index;
protected:
public:
	MenuController(int player_idx);

	virtual int GetPlayerIndex() {
		return player_index;
	}
	virtual int GetHitPoints() {
		return 1;
	};
	virtual bool CanTakeInput() {
		return true;
	};

	void MenuController::HandleLeftStickInput(float horizontal, float vertical);
	void MenuController::HandleRightStickInput(float horizontal, float vertical);
	void MenuController::HandleButtonAPress();
	void MenuController::HandleButtonARelease();
	void MenuController::HandleButtonBPress();
	void MenuController::HandleButtonBRelease();
	void MenuController::HandleButtonXPress();
	void MenuController::HandleButtonXRelease();
	void MenuController::HandleButtonYPress() { }

	void MenuController::HandleButtonYRelease() { }

	void MenuController::HandleButtonRightBumperPress() { }

	void MenuController::HandleButtonRightBumperRelease() { }

	void MenuController::HandleButtonLeftBumperPress() { }

	void MenuController::HandleButtonLeftBumperRelease() { }

	void MenuController::HandleButtonSelectPress();

	void MenuController::HandleButtonSelectRelease();

	void MenuController::HandleButtonStartPress();

	void MenuController::HandleButtonStartRelease();

	void MenuController::HandleDpadRightPress();

	void MenuController::HandleDpadRightRelease();

	void MenuController::HandleDpadLeftPress();

	void MenuController::HandleDpadLeftRelease();

	void MenuController::HandleDpadUpPress();

	void MenuController::HandleDpadUpRelease();

	void MenuController::HandleDpadDownPress();

	void MenuController::HandleDpadDownRelease();
};

#endif