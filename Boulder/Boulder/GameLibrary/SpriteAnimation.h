#ifndef SPRITE_ANIMATION_H
#define SPRITE_ANIMATION_H 

using namespace std;
#include <SFML/Graphics.hpp>
#include "AssetManager.h" 
#include "Singleton.h"

class SpriteAnimation {
private:
	int current_frame;
	bool facing_right;
	int sprite_frame_height;
	int sprite_frame_width;
	int number_of_frames;
	int number_of_columns;
	int number_of_rows;
	float sprite_scale;
	sf::Texture* texture;
	sf::Sprite* sprite;
	sf::IntRect texture_rect;
	bool looping;
public:
	sf::RenderWindow *render_window;
	SpriteAnimation(sf::RenderWindow* window, std::string file_path_and_name, int frame_width, int frame_height, int total_frames, int columns_count, int rows_count, float scale, sf::Color color, bool loop = true);
	void Draw(sf::Vector2f camera_position, sf::Vector2f parent_object_mid_position);
	void Flip();
	bool IsFacingRight();
	void SetColor(sf::Color new_color);
	int GetNumberOfFrames();
};

#endif