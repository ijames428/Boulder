#ifndef CAMERA_H
#define CAMERA_H

using namespace std;
#include <SFML/Graphics.hpp>

class Camera {
private:
	sf::Vector2f real_viewport_position;

	float lerp = 0.01f;
	sf::Int64 current_frame;
	bool screen_shaking;
	sf::Int64 screen_shake_start_time;
	sf::Int64 screen_shake_duration;
	float screen_shake_magnitude;
	sf::Vector2f viewport_position_with_screen_shake;
	sf::Vector2f screen_shake_amount;
public:
	sf::Vector2f viewport_position;
	sf::Vector2f viewport_dimensions;
	Camera(sf::Vector2f position, sf::Vector2f dimensions);
	void Update(sf::Int64 curr_frame, sf::Int64 delta_time, float player_body_position_x, float player_body_position_y);
	void ScreenShake(float magnitude);
};

#endif