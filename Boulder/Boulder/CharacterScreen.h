#ifndef CHARACTER_SCREEN_H
#define CHARACTER_SCREEN_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "SmashCharacter.h"

class CharacterScreen {
private:
	SmashCharacter* Player;
	sf::Text HitPointsText;
	sf::Text CharacterLevelText;
	sf::Text CharacterExperiencePointsText;
	sf::Text WeaponLevelText;
	sf::Text WeaponExperiencePointsText;
	sf::Text BonusesFromLevelsText;
	sf::Text RunesSectionText;
	sf::Text RuneDescriptionText;
	sf::Text CannotChangeRunesRightNowText;

	sf::RectangleShape* runeAreaBackgroundRect;

	sf::Texture* DpadTexture;
	sf::Sprite* DpadSprite;

	sf::Texture* WindowTexture;
	sf::Sprite* WindowSprite;

	sf::Texture* AButtonTexture;
	sf::Sprite* AButtonSprite;

	sf::Font ringbearerFont;

	sf::RenderWindow* render_window;
	Camera* camera;

	std::vector<sf::Text*> RuneTextsList;
	sf::Text RunesDescriptionText;

	int numberOfRunesToDisplayInList;
	int firstRuneBeingDisplayedInUiInternalRuneListIndex;
	int indexOfRuneBeingHoveredInInternalRuneList;
	int indexOfRuneBeingHoveredInUi;

	sf::RectangleShape* cursorRectangle;

	void ResetPositionOfRuneImagesAroundDpad();
public:
	CharacterScreen(SmashCharacter* player, sf::RenderWindow* rndr_win, Camera* cam);
	void Draw(sf::Vector2f camera_dimensions);
	void Open();
	void Close();
	bool IsOpen;
	void SwitchToAssigningRunes();
	void SwitchToCharacterStats();
	bool AreAnyEquippedRunesActive();
	void HandleDpadRightPress();
	void HandleDpadRightRelease();
	void HandleDpadLeftPress();
	void HandleDpadLeftRelease();
	void HandleDpadUpPress();
	void HandleDpadUpRelease();
	void HandleDpadDownPress();
	void HandleDpadDownRelease();
	void MoveCursorDown();
	void MoveCursorUp();
	bool IsOnAssigningRunesPage;
};

#endif