#pragma once

using namespace std;
#include "stdafx.h"
#include "Rune.h"  

Rune::Rune(string name, string description, string image_relative_file_path_and_name) {
	Name = name;
	Description = description;
	Active = false;

	Texture = Singleton<AssetManager>().Get()->GetTexture(image_relative_file_path_and_name);
	UiSprite = new sf::Sprite(*Texture);
	InGameSprite = new sf::Sprite(*Texture);
	InGameSprite->setScale(0.15f, 0.15f); 
	CharacterScreenSprite = new sf::Sprite(*Texture);
	CharacterScreenSprite->setScale(0.3f, 0.3f);
}