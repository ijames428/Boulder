#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "AssetManager.h"  

AssetManager::AssetManager() {
}

sf::Texture* AssetManager::GetTexture(std::string file_path) {
	std::map<std::string, sf::Texture*>::iterator result = textures.find(file_path);

	if (result != textures.end()) {
		return result->second;
	}
	else {
		sf::Texture* new_texture = new sf::Texture();

		if (!new_texture->loadFromFile(file_path)) {
			std::string str = "File not found: " + file_path;
			throw exception(str.c_str());
			return new_texture;
		}

		textures[file_path] = new_texture;
	}

	return textures[file_path];
}