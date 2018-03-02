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
#include "Menu.h"
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
float left_stick_vertical = 0.0f;

bool a_button_previous = false;
bool b_button_previous = false;
bool start_button_previous = false;

sf::Event event;

int frames_per_second = 60;
sf::Int64 microseconds_in_a_second = 1000000;
sf::Int64 microseconds_per_frame = microseconds_in_a_second / frames_per_second;
sf::Int64 current_frame = 0;

Menu* MainMenu;
bool can_take_another_left_stick_input_from_menu_controller = true;
bool load_game = false;

int good_frames = 0;
int bad_frames = 0;
bool playGameWithCommentary = false;

void NewGame() {
	MainMenu->Close();
	GameState = GAME_STATE_NEW_SINGLE_PLAYER;
	load_game = false;
	playGameWithCommentary = false;
}

void NewGameWithCommentary() {
	MainMenu->Close();
	GameState = GAME_STATE_NEW_SINGLE_PLAYER;
	load_game = false;
	playGameWithCommentary = true;
}

void LoadGame() {
	MainMenu->Close();
	GameState = GAME_STATE_NEW_SINGLE_PLAYER;
	load_game = true;
}

void Exit() {
	window->close();
}

int main()
{
	if (!sf::Shader::isAvailable())
	{
		cout << "----Shaders are not supported on this setup.\n";
	}

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
	
	float background_music_volume = 10.0f;
	float combat_music_volume = 0.0f;

	bool go_to_credits = false;

	Singleton<Settings>::Get()->effects_volume = 10.0f;
	Singleton<Settings>::Get()->music_volume = 50.0f;

	if (!background_music.openFromFile("Sound/background_music0.ogg"))
		return -1;
	background_music.setVolume((float)background_music_volume * (Singleton<Settings>::Get()->music_volume / 100.0f));
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

	camera = new Camera(sf::Vector2f(0, 0), sf::Vector2f(viewport_width, viewport_height));
	window = new sf::RenderWindow(sf::VideoMode::VideoMode((int)window_width, (int)window_height), "Shadhorimn");// , sf::Style::Fullscreen);
	window->setVerticalSyncEnabled(false);
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

	title_text = sf::Text("Shadhorimn 2", ringbearer_font, 90);
	title_text.setPosition(viewport_width / 2.0f - 260.0f, viewport_height / 2.0f - 200.0f);

	start_text = sf::Text("Press A/Start to Begin", ringbearer_font);
	start_text.setPosition(viewport_width / 2.0f - 120.0f, viewport_height / 2.0f - 60.0f);

	credits_text = sf::Text("Made by Ian James\n\n\n\n\nThank you for playing Shadhorimn", ringbearer_font);
	credits_text.setPosition(viewport_width / 2.0f - 220.0f, viewport_height + 50.0f);

	MainMenu = new Menu(window, camera->viewport_dimensions);
	MainMenu->AddItem("New Game", &NewGame);
	MainMenu->AddItem("Play Game With Audio Commentary", &NewGameWithCommentary);
	MainMenu->AddItem("Load Game", &LoadGame);
	MainMenu->AddItem("Exit", &Exit);

	while (window->isOpen())
	{
		time = clock.getElapsedTime();
		time_current = time.asMicroseconds();

		a_button_current = sf::Joystick::isButtonPressed(0, a_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
		b_button_current = sf::Joystick::isButtonPressed(0, b_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
		start_button_current = sf::Joystick::isButtonPressed(0, start_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return);

		left_stick_vertical = 0.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			left_stick_vertical = 100.0f;
		} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			left_stick_vertical = -100.0f;
		} else {
			left_stick_vertical = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
		}

		if (time_previous + microseconds_per_frame < time_current) {
			frame_delta = time_current - time_previous;
			time_previous = time_current;

			if (GameState == GAME_STATE_LOGOS) {
				UpdateGameStateLogos();
			} else if (GameState == GAME_STATE_START_MENU) {
				UpdateGameStateStartMenu();
			} else if (GameState == GAME_STATE_NEW_SINGLE_PLAYER) {
				GameState = GAME_STATE_INIT_SINGLE_PLAYER;
			} else if (GameState == GAME_STATE_INIT_SINGLE_PLAYER) {
				window->clear();
				UpdateGameStateLoadingScreen();
				Singleton<SmashWorld>::Get()->Init(window, camera, (float)frames_per_second);
				if (load_game) {
					Singleton<SmashWorld>::Get()->ImportSaveData();
				}
				if (playGameWithCommentary) {
					Singleton<SmashWorld>::Get()->StartAudioCommentary();
				}
				GameState = GAME_STATE_IN_SINGLE_PLAYER;
			} else if (GameState == GAME_STATE_IN_SINGLE_PLAYER) {
				go_to_credits = Singleton<SmashWorld>::Get()->Update(current_frame, frame_delta / 1000);

				if (go_to_credits) {
					GameState = GAME_STATE_CREDITS;
				} else if (Singleton<SmashWorld>::Get()->ShouldExitToMainMenu()) {
					MainMenu->Open();
					GameState = GAME_STATE_START_MENU;
				}

				HandleClosingEvent();
				SetPreviousButtonValues();
			} else if (GameState == GAME_STATE_CREDITS) {
				UpdateGameStateCredits();
			}

			current_frame++;

			if (microseconds_in_a_second / frame_delta == frames_per_second - 1) {
				good_frames++;
			} else {
				bad_frames++;
			}

			//std::printf("%4.2f pct good frames\t%4.2f pct bad frames.  \r", (((float)good_frames / (float)current_frame) * 100.0f), (((float)bad_frames / (float)current_frame) * 100.0f));
			//cout << (((float)good_frames / (float)current_frame) * 100.0f) << "% of frames are good\t\t" << (((float)bad_frames / (float)current_frame) * 100.0f) << "% of frames are bad\r";
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

	if (proceed) {
		MainMenu->Open();
		GameState = GAME_STATE_START_MENU;
	}

	HandleClosingEvent();
	SetPreviousButtonValues();

	window->display();
}

void UpdateGameStateStartMenu() {
	window->clear();

	if (MainMenu->IsOpen) {
		if (left_stick_vertical > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			MainMenu->MoveCursorDown();
			can_take_another_left_stick_input_from_menu_controller = false;
		}
		else if (left_stick_vertical < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			MainMenu->MoveCursorUp();
			can_take_another_left_stick_input_from_menu_controller = false;
		}
		else if (left_stick_vertical >= -90.0f && left_stick_vertical <= 90.0f) {
			can_take_another_left_stick_input_from_menu_controller = true;
		}
	}

	window->draw(title_text);
	//window->draw(start_text);
	MainMenu->Draw(current_frame);

	if (WasButtonAPressed() || WasButtonStartPressed()) {
		MainMenu->ExecuteCurrentSelection();
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
		MainMenu->Open();
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
		if (event.type == sf::Event::Closed)
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