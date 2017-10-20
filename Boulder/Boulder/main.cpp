#include "stdafx.h"
//using namespace std;
//#include <iostream>
//#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
//#include "..\PlatformerLibrary\Platform.h"
//#include "..\GameLibrary\Singleton.h"
#include "World.h"
#include "SmashWorld.h"
#include "GameLibrary\InputHandler.h"
//#include "PlayerCharacter.h"
//#include "Drone.h"
//#include "..\GameLibrary\Camera.h"
//#include "..\GameLibrary\Settings.h"

int IncrementTransparency(int transparency);
void SetPreviousButtonValues();
void UpdateGameStateLogos();
void UpdateGameStateStartMenu();
void UpdateGameStateLoadingScreen();
void UpdateGameStateCredits();

void HandleClosingEvent();
bool WasButtonAPressed();
bool WasButtonAReleased();
bool WasButtonBPressed();
bool WasButtonBReleased();
bool WasButtonStartPressed();
bool WasButtonStartReleased();
void SetFramesPerSecond(int new_fps);

enum GameStates
{
	GAME_STATE_LOGOS,
	GAME_STATE_START_MENU,
	GAME_STATE_NEW_SINGLE_PLAYER,
	GAME_STATE_INIT_SINGLE_PLAYER,
	GAME_STATE_IN_SINGLE_PLAYER,
	GAME_STATE_NEW_MULTIPLAYER,
	GAME_STATE_INIT_MULTIPLAYER,
	GAME_STATE_IN_MULTIPLAYER,
	GAME_STATE_CREDITS
};
GameStates GameState = GAME_STATE_LOGOS;

float viewport_width;
float viewport_height;
float window_width;
float window_height;

sf::RenderWindow* window;
Camera* camera;

//InputHandler* input_handler;
//PlayerCharacter* main_character;

sf::Music background_music;
sf::Music combat_music;

sf::Texture logo_screen_texture;
sf::Sprite logo_screen_sprite;
int logo_screen_sprite_transparency;
sf::Texture start_menu_background_texture;
sf::Sprite start_menu_background_sprite;
sf::Font ringbearer_font;
sf::Text title_text;
sf::Text start_text;

sf::Text credits_text;

int a_button = 0;
int b_button = 1;
int start_button = 7;

bool a_button_current;
bool b_button_current;
bool start_button_current;

bool a_button_previous = false;
bool b_button_previous = false;
bool start_button_previous = false;

sf::Event event;

int frames_per_second = 60;
sf::Int64 microseconds_in_a_second = 1000000;
sf::Int64 microseconds_per_frame = microseconds_in_a_second / frames_per_second;

int main()
{
	sf::Clock clock;
	sf::Time time;
	viewport_width = 1280.0f;
	viewport_height = 720.0f;
	window_width = viewport_width;
	window_height = viewport_height;

	time = clock.getElapsedTime();

	sf::Int64 time_current = time.asMicroseconds();
	sf::Int64 time_previous = time.asMicroseconds();
	sf::Int64 frame_delta;
	sf::Int64 start_time = time.asMicroseconds();
	sf::Int64 current_frame = 0;
	
	float background_music_volume = 100.0f;
	float combat_music_volume = 0.0f;

	Singleton<Settings>::Get()->effects_volume = 10.0f;
	Singleton<Settings>::Get()->music_volume = 50.0f;

	if (!background_music.openFromFile("Sound/background_music0.ogg"))
		return -1;
	combat_music.setVolume((float)background_music_volume * (Singleton<Settings>::Get()->music_volume / 100.0f));
#ifdef _DEBUG
#else
	background_music.play();
#endif
	background_music.setLoop(true);

	if (!combat_music.openFromFile("Sound/combat_music.ogg"))
		return -1;
	combat_music.setVolume((float)combat_music_volume * (Singleton<Settings>::Get()->music_volume / 100.0f));
#ifdef _DEBUG
#else
	combat_music.play();
#endif
	combat_music.setLoop(true);

	camera = new Camera(sf::Vector2f(viewport_width, viewport_height), sf::Vector2f(viewport_width, viewport_height));
	window = new sf::RenderWindow(sf::VideoMode::VideoMode((int)window_width, (int)window_height), "Shadhorimn");// , sf::Style::Fullscreen);
	//sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode::getDesktopMode(), "Shadhorimn");// , sf::Style::Fullscreen);
	//main_character = new PlayerCharacter(window, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(40.0f, 80.0f), true);
	//input_handler = new InputHandler(main_character);

	if (!logo_screen_texture.loadFromFile("Images/LogoScreen.png"))
		return -1;

	logo_screen_sprite = sf::Sprite(logo_screen_texture);
	int logo_screen_sprite_transparency = 0;

	if (!start_menu_background_texture.loadFromFile("Images/StartMenuBackground.png"))
		return -1;

	start_menu_background_sprite = sf::Sprite(start_menu_background_texture);

	if (!ringbearer_font.loadFromFile("Images/RingbearerFont.ttf"))
		return -1;

	title_text = sf::Text("Shadhorimn", ringbearer_font, 90);
	title_text.setPosition(viewport_width / 2.0f - 260.0f, viewport_height / 2.0f - 200.0f);

	start_text = sf::Text("Press A to PvE\nPress Start to PvP", ringbearer_font);
	start_text.setPosition(viewport_width / 2.0f - 120.0f, viewport_height / 2.0f - 60.0f);

	credits_text = sf::Text("Made by Ian James\n\n\n\n\nThank you for playing Shadhorimn", ringbearer_font);
	credits_text.setPosition(viewport_width / 2.0f - 220.0f, viewport_height + 50.0f);

	while (window->isOpen())
	{
		time = clock.getElapsedTime();
		time_current = time.asMicroseconds();

		a_button_current = sf::Joystick::isButtonPressed(0, a_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
		b_button_current = sf::Joystick::isButtonPressed(0, b_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
		start_button_current = sf::Joystick::isButtonPressed(0, start_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return);

		if (time_previous + microseconds_per_frame < time_current) {
			frame_delta = time_current - time_previous;
			time_previous = time_current;

			if (GameState == GAME_STATE_LOGOS) {
				UpdateGameStateLogos();
			} else if (GameState == GAME_STATE_START_MENU) {
				UpdateGameStateStartMenu();
			} else if (GameState == GAME_STATE_NEW_MULTIPLAYER) {
				GameState = GAME_STATE_INIT_MULTIPLAYER;
			} else if (GameState == GAME_STATE_INIT_MULTIPLAYER) {
				window->clear();
				UpdateGameStateLoadingScreen();
				Singleton<SmashWorld>::Get()->Init(window, camera);
				GameState = GAME_STATE_IN_MULTIPLAYER;
				//input_handler->EatInputsForNumberOfFrames(1);
			} else if (GameState == GAME_STATE_IN_MULTIPLAYER) {
				Singleton<SmashWorld>::Get()->Update(current_frame, frame_delta / 1000);

				if (Singleton<SmashWorld>::Get()->ShouldExitMultiplayer()) {
					GameState = GAME_STATE_START_MENU;
				}

				HandleClosingEvent();
				SetPreviousButtonValues();
			} else if (GameState == GAME_STATE_NEW_SINGLE_PLAYER) {
				Singleton<World>::Get()->StartNewGame();
				GameState = GAME_STATE_INIT_SINGLE_PLAYER;
			} else if (GameState == GAME_STATE_INIT_SINGLE_PLAYER) {
				window->clear();
				UpdateGameStateLoadingScreen();
				//Singleton<World>::Get()->Init(window, camera, main_character);
				GameState = GAME_STATE_IN_SINGLE_PLAYER;
				//input_handler->EatInputsForNumberOfFrames(1);
			} else if (GameState == GAME_STATE_IN_SINGLE_PLAYER) {
				if (start_button_current && !start_button_previous) {
					Singleton<World>::Get()->paused = !Singleton<World>::Get()->paused;
				}
				if (Singleton<World>::Get()->paused) {
					//input_handler->EatInputsForNumberOfFrames(1);
				}

				//input_handler->Update();
				Singleton<World>::Get()->Update(current_frame, frame_delta / 1000);

				if (Singleton<World>::Get()->IsPlayerInCombat()) {
					if (combat_music_volume < 100) {
						combat_music_volume++;
						background_music_volume--;

						combat_music.setVolume((float)combat_music_volume * (Singleton<Settings>::Get()->music_volume / 100.0f));
						background_music.setVolume((float)background_music_volume * (Singleton<Settings>::Get()->music_volume / 100.0f));
					}
				} else {
					if (background_music_volume < 100) {
						background_music_volume++;
						combat_music_volume--;

						background_music.setVolume((float)background_music_volume * (Singleton<Settings>::Get()->music_volume / 100.0f));
						combat_music.setVolume((float)combat_music_volume * (Singleton<Settings>::Get()->music_volume / 100.0f));
					}
				}
				
				if (Singleton<World>::Get()->CanContinue()) {
					SetFramesPerSecond(60);
					if ((a_button_current && !a_button_previous) || (start_button_current && !start_button_previous)) {
						if (Singleton<World>::Get()->current_number_of_lives > 0) {
							Singleton<World>::Get()->current_number_of_lives--;
							GameState = GAME_STATE_INIT_SINGLE_PLAYER;
						} else {
							Singleton<World>::Get()->current_number_of_lives = 2;
							GameState = GAME_STATE_START_MENU;
						}
					}
				} else if (Singleton<World>::Get()->main_character->hit_points <= 0) {
					SetFramesPerSecond(10);
				} else if (Singleton<World>::Get()->DidThePlayerBeatTheGame()) {
					SetFramesPerSecond(10);
					//input_handler->EatInputsForNumberOfFrames(1);
					
					if (Singleton<World>::Get()->ShouldGoToCredits()) {
						SetFramesPerSecond(60);
						GameState = GAME_STATE_CREDITS;
						credits_text.setPosition(viewport_width / 2.0f - 120.0f, viewport_height + 50.0f);
					}
				} 

				HandleClosingEvent();
				SetPreviousButtonValues();
			} else if (GameState == GAME_STATE_CREDITS) {
				UpdateGameStateCredits();
			}

			current_frame++;
		}
	}

	return 0;
}

void SetFramesPerSecond(int new_fps) {
	frames_per_second = new_fps;
	microseconds_per_frame = microseconds_in_a_second / frames_per_second;
}

void UpdateGameStateLogos() {
	window->clear();

	bool proceed = false;

	logo_screen_sprite_transparency = IncrementTransparency(logo_screen_sprite_transparency);
	logo_screen_sprite.setColor(sf::Color(255, 255, 255, logo_screen_sprite_transparency));
	window->draw(logo_screen_sprite);

	if (WasButtonAPressed() || WasButtonStartPressed()) {
		proceed = true;
	}

	if (proceed)
		GameState = GAME_STATE_START_MENU;

	HandleClosingEvent();
	SetPreviousButtonValues();

	window->display();
}

void UpdateGameStateStartMenu() {
	window->clear();

	window->draw(title_text);
	window->draw(start_text);

	if (WasButtonAPressed()) {
		GameState = GAME_STATE_NEW_SINGLE_PLAYER;
	}
	if (WasButtonStartPressed()) {
		GameState = GAME_STATE_NEW_MULTIPLAYER;
	}

	HandleClosingEvent();
	SetPreviousButtonValues();

	window->display();
}

void UpdateGameStateLoadingScreen() {
	window->clear();

	HandleClosingEvent();
	SetPreviousButtonValues();

	window->display();
}

void UpdateGameStateCredits() {
	window->clear();

	credits_text.setPosition(credits_text.getPosition().x, credits_text.getPosition().y - 1.0f);
	window->draw(credits_text);

	if (credits_text.getPosition().y < - 350.0f) {
		GameState = GAME_STATE_START_MENU;
	}

	HandleClosingEvent();
	SetPreviousButtonValues();

	window->display();
}

int IncrementTransparency(int transparency) {
	if (transparency >= 255) {
		transparency = 255;
	}
	else {
		transparency++;
	}

	return transparency;
}

void SetPreviousButtonValues() {
	a_button_previous = a_button_current;
	b_button_previous = b_button_current;
	start_button_previous = start_button_current;
}

void HandleClosingEvent() {
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed || event.key.code == sf::Keyboard::Escape)
			window->close();
	}
}

bool WasButtonAPressed() {
	return a_button_current && !a_button_previous;
}

bool WasButtonAReleased() {
	return !a_button_current && a_button_previous;
}

bool WasButtonBPressed() {
	return b_button_current && !b_button_previous;
}

bool WasButtonBReleased() {
	return !b_button_current && b_button_previous;
}

bool WasButtonStartPressed() {
	return start_button_current && !start_button_previous;
}

bool WasButtonStartReleased() {
	return !start_button_current && start_button_previous;
}