#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

using namespace std;
#include <SFML/Graphics.hpp>

class AssetManager {
private:
	std::map<std::string, sf::Texture*> textures = std::map<std::string, sf::Texture*>();
public:
	AssetManager();
	sf::Texture* GetTexture(std::string file_path);
};

#endif