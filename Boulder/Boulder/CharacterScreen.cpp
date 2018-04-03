#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "CharacterScreen.h"

CharacterScreen::CharacterScreen(SmashCharacter* player, sf::RenderWindow* rndr_win, Camera* cam) {
	render_window = rndr_win;
	camera = cam;

	IsOpen = false;
	IsOnAssigningRunesPage = false;

	numberOfRunesToDisplayInList = 3;
	indexOfRuneBeingHoveredInInternalRuneList = 0;
	indexOfRuneBeingHoveredInUi = 0;
	firstRuneBeingDisplayedInUiInternalRuneListIndex = 0;

	Player = player;

	ringbearerFont.loadFromFile("Images/RingbearerFont.ttf");

	HitPointsText = sf::Text("", ringbearerFont, 45);
	CharacterLevelText = sf::Text("", ringbearerFont, 45);
	CharacterExperiencePointsText = sf::Text("", ringbearerFont, 45);
	//WeaponLevelText = sf::Text("", ringbearerFont, 45);
	//WeaponExperiencePointsText = sf::Text("", ringbearerFont, 45);
	//BonusesFromLevelsText = sf::Text("", ringbearerFont, 45);
	RunesSectionText = sf::Text("", ringbearerFont, 45);

	HitPointsText.setFillColor(sf::Color::White);
	CharacterLevelText.setFillColor(sf::Color::White);
	CharacterExperiencePointsText.setFillColor(sf::Color::White);
	//WeaponLevelText.setFillColor(sf::Color::White);
	//WeaponExperiencePointsText.setFillColor(sf::Color::White);
	//BonusesFromLevelsText.setFillColor(sf::Color::White);
	RunesSectionText.setFillColor(sf::Color::Black);

	HitPointsText.setPosition(10.0f, 100.0f);
	CharacterLevelText.setPosition(10.0f, 150.0f);
	CharacterExperiencePointsText.setPosition(500.0f, 150.0f);
	//WeaponLevelText.setPosition(10.0f, 200.0f);
	//WeaponExperiencePointsText.setPosition(500.0f, 200.0f);
	//BonusesFromLevelsText.setPosition(800.0f, 100.0f);
	RunesSectionText.setPosition(800.0f, 400.0f);
	RunesSectionText.setString("Runes");

	DpadTexture = Singleton<AssetManager>().Get()->GetTexture("Images/Runes/Dpad.png");
	DpadSprite = new sf::Sprite(*DpadTexture);
	DpadSprite->setPosition(camera->viewport_dimensions.x - camera->viewport_dimensions.x / 4.0f - DpadTexture->getSize().x / 2.0f, camera->viewport_dimensions.y - 150.0f);

	WindowTexture = Singleton<AssetManager>().Get()->GetTexture("Images/StartMenuBackground.png");
	WindowSprite = new sf::Sprite(*WindowTexture);
	WindowSprite->setPosition(camera->viewport_dimensions.x / 2.0f - WindowTexture->getSize().x / 2.0f, camera->viewport_dimensions.y / 2.0f - WindowTexture->getSize().y / 2.0f);

	runeAreaBackgroundRect = new sf::RectangleShape(sf::Vector2f(camera->viewport_dimensions.x / 2.0f - 10.0f, camera->viewport_dimensions.y / 2.0f - 10.0f));
	runeAreaBackgroundRect->setPosition(sf::Vector2f(camera->viewport_dimensions.x / 2.0f - 10.0f, camera->viewport_dimensions.y / 2.0f - 10.0f));

	AButtonTexture = Singleton<AssetManager>().Get()->GetTexture("Images/AButton.png");
	AButtonSprite = new sf::Sprite(*AButtonTexture);
	AButtonSprite->setPosition(790.0f, 400.0f);

	int runesListSize = (int)Player->OwnedRunesList.size();
	for (int i = 0; i < runesListSize; i++) {
		RuneTextsList.push_back(new sf::Text("(" + to_string(i + 1) + "/" + to_string(runesListSize) + ") " + Player->OwnedRunesList[i]->Name, ringbearerFont, 45));
		RuneTextsList[i]->setFillColor(sf::Color::White);
	}

	cursorRectangle = new sf::RectangleShape(sf::Vector2f(500.0f, 55.0f));
	cursorRectangle->setOutlineThickness(5.0f);
	cursorRectangle->setFillColor(sf::Color::Black);
	cursorRectangle->setOutlineColor(sf::Color::White);

	CannotChangeRunesRightNowText = sf::Text("Cannot reassign Runes while some\nare active.", ringbearerFont, 30);
	CannotChangeRunesRightNowText.setFillColor(sf::Color::Red);
	CannotChangeRunesRightNowText.setPosition(10.0f, camera->viewport_dimensions.y - 100.0f);

	RuneDescriptionText = sf::Text("", ringbearerFont, 25);
	RuneDescriptionText.setFillColor(sf::Color::White);
	RuneDescriptionText.setPosition(700.0f, 100.0f);

	if (runesListSize > indexOfRuneBeingHoveredInInternalRuneList) {
		RuneDescriptionText.setString(Player->OwnedRunesList[indexOfRuneBeingHoveredInInternalRuneList]->Description);
	} else {
		RuneDescriptionText.setString("");
	}

	scrollBarLengthRectangle = new sf::RectangleShape(sf::Vector2f(4.0f, 50.0f * numberOfRunesToDisplayInList));
	scrollBarLengthRectangle->setPosition(10.0f, 50.0f);
	scrollBarLengthRectangle->setFillColor(sf::Color::White);

	scrollBarCursorRectangle = new sf::RectangleShape(sf::Vector2f(10.0f, 40.0f));
	scrollBarCursorRectangle->setPosition(10.0f - 3.0f, 50.0f);
	scrollBarCursorRectangle->setFillColor(sf::Color::White);

	GoToAssignRunesPageText = sf::Text("Press A to go to the Assign Runes page.\nPress B to resume game.", ringbearerFont, 15);
	GoToAssignRunesPageText.setFillColor(sf::Color::Yellow);
	GoToAssignRunesPageText.setPosition(900.0f, 5.0f);

	GoBackToCharacterStatsPageText = sf::Text("Press B to go back to the Character\nStats page.", ringbearerFont, 15);
	GoBackToCharacterStatsPageText.setFillColor(sf::Color::Yellow);
	GoBackToCharacterStatsPageText.setPosition(900.0f, 5.0f);

	InstructionsText = sf::Text("Hit Left, Up, or Right on the Dpad\nto assign the currently hovered\nrune to that direction.", ringbearerFont, 15);
	InstructionsText.setFillColor(sf::Color::Cyan);
	InstructionsText.setPosition(600.0f, 5.0f);
}

void CharacterScreen::Draw(sf::Vector2f camera_dimensions) {
	if (IsOnAssigningRunesPage) {
		if (AreAnyEquippedRunesActive()) {
			render_window->draw(CannotChangeRunesRightNowText);
		}
		render_window->draw(*runeAreaBackgroundRect);
		render_window->draw(*cursorRectangle);
		render_window->draw(RuneDescriptionText);
		render_window->draw(*scrollBarLengthRectangle);
		render_window->draw(*scrollBarCursorRectangle);
		render_window->draw(InstructionsText);
		render_window->draw(GoBackToCharacterStatsPageText);

		int runeTextsListSize = (int)RuneTextsList.size();
		for (int i = firstRuneBeingDisplayedInUiInternalRuneListIndex; i < firstRuneBeingDisplayedInUiInternalRuneListIndex + numberOfRunesToDisplayInList; i++) {
			if (i >= runeTextsListSize) {
				break;
			}

			RuneTextsList[i]->setPosition(sf::Vector2f(30.0f, 50.0f + 50.0f * (i - firstRuneBeingDisplayedInUiInternalRuneListIndex)));
			render_window->draw(*RuneTextsList[i]);
		}

		cursorRectangle->setPosition(sf::Vector2f(30.0f, 55.0f + 50.0f * indexOfRuneBeingHoveredInUi));

		render_window->draw(RunesSectionText);
		render_window->draw(*DpadSprite);

		if (Player->DpadLeftRune != nullptr) {
			render_window->draw(*Player->DpadLeftRune->CharacterScreenSprite);
		}
		if (Player->DpadUpRune != nullptr) {
			render_window->draw(*Player->DpadUpRune->CharacterScreenSprite);
		}
		if (Player->DpadRightRune != nullptr) {
			render_window->draw(*Player->DpadRightRune->CharacterScreenSprite);
		}
	} else {
		render_window->draw(*WindowSprite);
		render_window->draw(*runeAreaBackgroundRect);

		render_window->draw(HitPointsText);
		render_window->draw(CharacterLevelText);
		render_window->draw(CharacterExperiencePointsText);
		//render_window->draw(WeaponLevelText);
		//render_window->draw(WeaponExperiencePointsText);
		//render_window->draw(BonusesFromLevelsText);
		render_window->draw(RunesSectionText);
		render_window->draw(*DpadSprite);

		if (Player->DpadLeftRune != nullptr) {
			render_window->draw(*Player->DpadLeftRune->CharacterScreenSprite);
		}
		if (Player->DpadUpRune != nullptr) {
			render_window->draw(*Player->DpadUpRune->CharacterScreenSprite);
		}
		if (Player->DpadRightRune != nullptr) {
			render_window->draw(*Player->DpadRightRune->CharacterScreenSprite);
		}

		render_window->draw(*AButtonSprite);

		render_window->draw(GoToAssignRunesPageText);
	}
}

void CharacterScreen::Open() {
	IsOpen = true;

	ResetPositionOfRuneImagesAroundDpad();

	HitPointsText.setString("Hit Points: " + to_string(Player->GetHitPoints()) + "/" + to_string(Player->GetMaxHitPoints()));
	CharacterLevelText.setString("Character Level: " + to_string(Player->GetCharacterLevel()));
	CharacterExperiencePointsText.setString("Character XP: " + to_string(Player->GetExperienceTowardsNextCharacterLevel()) + "/" + to_string(Player->CharacterExperienceNeededForNextLevel(Player->GetCharacterLevel())));
	//WeaponLevelText.setString("Weapon Level: " + to_string(Player->GetWeaponLevel()));
	//WeaponExperiencePointsText.setString("Weapon XP: " + to_string(Player->GetExperienceTowardsNextWeaponLevel()) + "/" + to_string(Player->WeaponExperienceNeededForNextLevel(Player->GetWeaponLevel())));
}

void CharacterScreen::Close() {
	if (IsOnAssigningRunesPage) {
		IsOnAssigningRunesPage = false;
	} else {
		IsOpen = false;
	}
}

void CharacterScreen::SwitchToAssigningRunes() {
	IsOnAssigningRunesPage = true;
	indexOfRuneBeingHoveredInInternalRuneList = 0;
	indexOfRuneBeingHoveredInUi = 0;

	int runesListSize = (int)Player->OwnedRunesList.size();
	for (int i = 0; i < runesListSize; i++) {
		RuneTextsList.push_back(new sf::Text("(" + to_string(i + 1) + "/" + to_string(runesListSize) + ") " + Player->OwnedRunesList[i]->Name, ringbearerFont, 45));
		RuneTextsList[i]->setFillColor(sf::Color::White);
	}

	if (runesListSize > indexOfRuneBeingHoveredInInternalRuneList) {
		RuneDescriptionText.setString(Player->OwnedRunesList[indexOfRuneBeingHoveredInInternalRuneList]->Description);
	} else {
		RuneDescriptionText.setString("");
	}
}

void CharacterScreen::SwitchToCharacterStats() {
	IsOnAssigningRunesPage = false;
}

void CharacterScreen::MoveCursorDown() {
	indexOfRuneBeingHoveredInInternalRuneList++;
	bool cycle_to_top_of_list = false;
	int runesListSize = (int)Player->OwnedRunesList.size();

	if (indexOfRuneBeingHoveredInInternalRuneList >= runesListSize) {
		indexOfRuneBeingHoveredInInternalRuneList = 0;
		cycle_to_top_of_list = true;
	}

	if (indexOfRuneBeingHoveredInUi + 1 >= numberOfRunesToDisplayInList) {
		if (cycle_to_top_of_list) {
			indexOfRuneBeingHoveredInUi = 0;
			firstRuneBeingDisplayedInUiInternalRuneListIndex = 0;
		} else {
			firstRuneBeingDisplayedInUiInternalRuneListIndex++;
		}
	} else {
		indexOfRuneBeingHoveredInUi++;
	}

	if (runesListSize > indexOfRuneBeingHoveredInInternalRuneList) {
		RuneDescriptionText.setString(Player->OwnedRunesList[indexOfRuneBeingHoveredInInternalRuneList]->Description);
	} else {
		RuneDescriptionText.setString("");
	}

	scrollBarCursorRectangle->setPosition(10.0f - 3.0f, 50.0f + (((50.0f * numberOfRunesToDisplayInList)/* - 40.0f*/) * indexOfRuneBeingHoveredInInternalRuneList / (runesListSize > 0 ? runesListSize : 1)));
}

void CharacterScreen::MoveCursorUp() {
	indexOfRuneBeingHoveredInInternalRuneList--;
	bool cycle_to_bottom_of_list = false;
	int runesListSize = (int)Player->OwnedRunesList.size();

	if (indexOfRuneBeingHoveredInInternalRuneList < 0) {
		indexOfRuneBeingHoveredInInternalRuneList = (int)Player->OwnedRunesList.size() - 1;
		cycle_to_bottom_of_list = true;
	}

	if (indexOfRuneBeingHoveredInUi <= 0) {
		if (cycle_to_bottom_of_list) {
			indexOfRuneBeingHoveredInUi = numberOfRunesToDisplayInList - 1;
			firstRuneBeingDisplayedInUiInternalRuneListIndex = (int)Player->OwnedRunesList.size() - numberOfRunesToDisplayInList;
		} else {
			firstRuneBeingDisplayedInUiInternalRuneListIndex--;
		}
	} else {
		indexOfRuneBeingHoveredInUi--;
	}

	if (runesListSize > indexOfRuneBeingHoveredInInternalRuneList) {
		RuneDescriptionText.setString(Player->OwnedRunesList[indexOfRuneBeingHoveredInInternalRuneList]->Description);
	} else {
		RuneDescriptionText.setString("");
	}

	scrollBarCursorRectangle->setPosition(10.0f - 3.0f, 50.0f + (((50.0f * numberOfRunesToDisplayInList)/* - 40.0f*/) * indexOfRuneBeingHoveredInInternalRuneList / (runesListSize > 0 ? runesListSize : 1)));
}

void CharacterScreen::HandleDpadRightPress() {
	if (AreAnyEquippedRunesActive()) {
		return;
	}

	if (IsOnAssigningRunesPage && (int)Player->OwnedRunesList.size() > indexOfRuneBeingHoveredInInternalRuneList) {
		Rune* rune_being_assigned = Player->OwnedRunesList[indexOfRuneBeingHoveredInInternalRuneList];

		if (rune_being_assigned->Name == Player->DpadLeftRune->Name) {
			Player->DpadLeftRune = Player->DpadRightRune;
		} else if (rune_being_assigned->Name == Player->DpadUpRune->Name) {
			Player->DpadUpRune = Player->DpadRightRune;
		}

		Player->DpadRightRune = rune_being_assigned;
		ResetEquippedValuesOnRunes();
		ResetPositionOfRuneImagesAroundDpad();
	}
}

void CharacterScreen::HandleDpadRightRelease() {
}

void CharacterScreen::HandleDpadLeftPress() {
	if (AreAnyEquippedRunesActive()) {
		return;
	}

	if (IsOnAssigningRunesPage && (int)Player->OwnedRunesList.size() > indexOfRuneBeingHoveredInInternalRuneList) {
		Rune* rune_being_assigned = Player->OwnedRunesList[indexOfRuneBeingHoveredInInternalRuneList];

		if (rune_being_assigned->Name == Player->DpadRightRune->Name) {
			Player->DpadRightRune = Player->DpadLeftRune;
		}
		else if (rune_being_assigned->Name == Player->DpadUpRune->Name) {
			Player->DpadUpRune = Player->DpadLeftRune;
		}

		Player->DpadLeftRune = rune_being_assigned;
		ResetEquippedValuesOnRunes();
		ResetPositionOfRuneImagesAroundDpad();
	}
}

void CharacterScreen::HandleDpadLeftRelease() {
}

void CharacterScreen::HandleDpadUpPress() {
	if (AreAnyEquippedRunesActive()) {
		return;
	}

	if (IsOnAssigningRunesPage && (int)Player->OwnedRunesList.size() > indexOfRuneBeingHoveredInInternalRuneList) {
		Rune* rune_being_assigned = Player->OwnedRunesList[indexOfRuneBeingHoveredInInternalRuneList];

		if (rune_being_assigned->Name == Player->DpadRightRune->Name) {
			Player->DpadRightRune = Player->DpadUpRune;
		}
		else if (rune_being_assigned->Name == Player->DpadLeftRune->Name) {
			Player->DpadLeftRune = Player->DpadUpRune;
		}

		Player->DpadUpRune = rune_being_assigned;
		ResetEquippedValuesOnRunes();
		ResetPositionOfRuneImagesAroundDpad();
	}
}

void CharacterScreen::HandleDpadUpRelease() {
}

void CharacterScreen::HandleDpadDownPress() {
}

void CharacterScreen::HandleDpadDownRelease() {
}

void CharacterScreen::ResetPositionOfRuneImagesAroundDpad() {
	if (Player->DpadLeftRune != nullptr) {
		Player->DpadLeftRune->CharScreenPosition = sf::Vector2f(camera->viewport_dimensions.x - 410.0f, camera->viewport_dimensions.y - 115.0f);
		Player->DpadLeftRune->CharacterScreenSprite->setPosition(Player->DpadLeftRune->CharScreenPosition);
	}

	if (Player->DpadUpRune != nullptr) {
		Player->DpadUpRune->CharScreenPosition = sf::Vector2f(camera->viewport_dimensions.x - 335.0f, camera->viewport_dimensions.y - 190.0f);
		Player->DpadUpRune->CharacterScreenSprite->setPosition(Player->DpadUpRune->CharScreenPosition);
	}

	if (Player->DpadRightRune != nullptr) {
		Player->DpadRightRune->CharScreenPosition = sf::Vector2f(camera->viewport_dimensions.x - 260.0f, camera->viewport_dimensions.y - 115.0f);
		Player->DpadRightRune->CharacterScreenSprite->setPosition(Player->DpadRightRune->CharScreenPosition);
	}
}

void CharacterScreen::ResetEquippedValuesOnRunes() {
	int runesListSize = (int)Player->OwnedRunesList.size();
	for (int i = 0; i < runesListSize; i++) {
		Player->OwnedRunesList[i]->Equipped = false;
	}

	if (Player->DpadLeftRune != nullptr) {
		Player->DpadLeftRune->Equipped = true;
	}

	if (Player->GetNumberOfRuneSlotsFromLevel(Player->GetCharacterLevel()) >= 3) {
		if (Player->DpadUpRune != nullptr) {
			Player->DpadUpRune->Equipped = true;
		}
		if (Player->DpadRightRune != nullptr) {
			Player->DpadRightRune->Equipped = true;
		}
	}
	else if (Player->GetNumberOfRuneSlotsFromLevel(Player->GetCharacterLevel()) >= 2) {
		if (Player->DpadUpRune != nullptr) {
			Player->DpadUpRune->Equipped = true;
		}
	}
}

bool CharacterScreen::AreAnyEquippedRunesActive() {
	return (Player->DpadLeftRune != nullptr && Player->DpadLeftRune->Active) ||
			(Player->DpadUpRune != nullptr && Player->DpadUpRune->Active) ||
			(Player->DpadRightRune != nullptr && Player->DpadRightRune->Active);
}