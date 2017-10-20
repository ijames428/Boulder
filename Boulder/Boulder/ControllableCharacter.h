#pragma once

class ControllableCharacter {
public:
	virtual int GetPlayerIndex() = 0;
	virtual int GetHitPoints() = 0;
	virtual bool CanTakeInput() = 0;
	virtual void HandleLeftStickInput(float horizontal, float vertical) = 0;
	virtual void HandleRightStickInput(float horizontal, float vertical) = 0;
	virtual void HandleButtonAPress() = 0;
	virtual void HandleButtonARelease() = 0;
	virtual void HandleButtonBPress() = 0;
	virtual void HandleButtonBRelease() = 0;
	virtual void HandleButtonXPress() = 0;
	virtual void HandleButtonXRelease() = 0;
	virtual void HandleButtonYPress() = 0;
	virtual void HandleButtonYRelease() = 0;
	virtual void HandleButtonRightBumperPress() = 0;
	virtual void HandleButtonRightBumperRelease() = 0;
	virtual void HandleButtonLeftBumperPress() = 0;
	virtual void HandleButtonLeftBumperRelease() = 0;
	virtual void HandleButtonSelectPress() = 0;
	virtual void HandleButtonSelectRelease() = 0;
	virtual void HandleButtonStartPress() = 0;
	virtual void HandleButtonStartRelease() = 0;
};