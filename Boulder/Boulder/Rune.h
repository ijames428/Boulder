#ifndef RUNE_H
#define RUNE_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "GameLibrary\AssetManager.h" 
#include "GameLibrary\Singleton.h"

class Rune {
public:
	Rune(string name, string description, string image_relative_file_path_and_name);
	string Name;
	string Description;
	bool Active;
	bool Equipped;
	sf::Texture* Texture;
	sf::Sprite* UiSprite;
	sf::Sprite* InGameSprite;
	sf::Sprite* CharacterScreenSprite;
	sf::Vector2f UiPosition;
	sf::Vector2f CharScreenPosition;
};

#endif