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
#include <fstream>
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
void UpdateGameStateNewSinglePlayer();

void HandleClosingEvent();
bool WasButtonAPressed();
bool WasButtonAReleased();
bool WasButtonBPressed();
bool WasButtonBReleased();
bool WasButtonYPressed();
bool WasButtonYReleased();
bool WasButtonStartPressed();
bool WasButtonStartReleased();
void SetFramesPerSecond(int new_fps);
void ExtractSavedGameDisplayInformation(string file_path, string* stage_of_game, string* character_level);
void BuildMainMenu();
void BuildLoadMenu();

void SaveSettings();

typedef void(*callback_function_with_param)(int); // type for conciseness

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

string menuMusicFileName = "Sound/MenuMusic.wav";
string combatMusicFileName = "Sound/CombatMusic.wav";
string travelingMusicFileName = "Sound/SkateBoardingMusic.wav";
string downTimeMusicFileName = "Sound/DownTimeMusic.wav";
sf::Music* menuMusic;
//sf::Music* combatMusic;
//sf::Music* travelingMusic;
//sf::Music* downTimeMusic;

sf::Texture logo_screen_texture;
sf::Sprite logo_screen_sprite;
int logo_screen_sprite_transparency;
sf::Texture start_menu_background_texture;
sf::Sprite start_menu_background_sprite;
sf::Font ringbearer_font;
sf::Text title_text;
sf::Text start_text;
sf::Text how_to_delete_save_text;
sf::Text credits_text;
sf::Text story_text;

int a_button = 0;
int b_button = 1; 
int y_button = 3;
int start_button = 7;

bool a_button_current;
bool b_button_current;
bool y_button_current;
bool start_button_current;
float left_stick_vertical = 0.0f;
float left_stick_horizontal = 0.0f;

bool a_button_previous = false;
bool b_button_previous = false;
bool y_button_previous = false;
bool start_button_previous = false;

sf::Event event;

int frames_per_second = 60;
sf::Int64 microseconds_in_a_second = 1000000;
sf::Int64 microseconds_per_frame = microseconds_in_a_second / frames_per_second;
sf::Int64 current_frame = 0;

Menu* MainMenu;
Menu* OptionsMenu;
Menu* LoadGameMenu;
Menu* ConfirmDeleteSaveMenu;
bool can_take_another_left_stick_input_from_menu_controller = true;
bool load_game = false;
int selected_save_slot = -1;
int first_empty_save_slot_found = -1;
int last_used_save_slot = 99;
int maxNumberOfSaveSlots = 10;

int good_frames = 0;
int bad_frames = 0;
bool playGameWithCommentary = false;

int numberOfSouls = 200;
std::vector<sf::CircleShape*> souls;
sf::Vector2f soulOriginPoint = sf::Vector2f(0.0f, 0.0f);
std::vector<sf::Vector2f*> soulVectors;
std::vector<float> soulAlphas;

void NewGame() {
	if (first_empty_save_slot_found < maxNumberOfSaveSlots && first_empty_save_slot_found > -1) {
		MainMenu->Close();
		GameState = GAME_STATE_NEW_SINGLE_PLAYER;
		story_text.setPosition(viewport_width / 2.0f - 220.0f, viewport_height + 50.0f);
		load_game = false;
		playGameWithCommentary = false;
		selected_save_slot = first_empty_save_slot_found;
	}
}

void NewGameWithCommentary() {
	MainMenu->Close();
	GameState = GAME_STATE_NEW_SINGLE_PLAYER;
	story_text.setPosition(viewport_width / 2.0f - 220.0f, viewport_height + 50.0f);
	load_game = false;
	playGameWithCommentary = true;
}

void LoadGameOfLastUsedSavedSlot() {
	MainMenu->Close();
	GameState = GAME_STATE_INIT_SINGLE_PLAYER;
	load_game = true;
	selected_save_slot = last_used_save_slot;
}

void LoadGame(int save_slot) {
	MainMenu->Close();
	GameState = GAME_STATE_INIT_SINGLE_PLAYER;
	load_game = true;
	selected_save_slot = save_slot;
}

void ExitConfirmDeleteSaveMenu() {
	BuildLoadMenu();
	ConfirmDeleteSaveMenu->Close();
	LoadGameMenu->Open();
}

void DeleteSave() {
	string save_data_file_name = "save" + to_string(selected_save_slot) + ".sav";
	if (remove(save_data_file_name.c_str()) != 0) {
		cout << "Error deleting file " << save_data_file_name << "\n";
	} else {
		if (last_used_save_slot == selected_save_slot) {
			last_used_save_slot = 99;
			SaveSettings();
		}
		BuildLoadMenu();
		BuildMainMenu();
	}

	ExitConfirmDeleteSaveMenu();
}

void Nothing() {
}

void OpenLoadGameMenu() {
	MainMenu->Close();
	LoadGameMenu->Open();
}

void ExitLoadGameMenu() {
	LoadGameMenu->Close();
	MainMenu->Open();
}

void Exit() {
	window->close();
}

void OpenOptionsMenu() {
	MainMenu->Close();

	OptionsMenu->SetCurrentSliderValueByText("Music Volume", (int)Singleton<Settings>::Get()->music_volume);
	OptionsMenu->SetCurrentSliderValueByText("Effects Volume", (int)Singleton<Settings>::Get()->effects_volume);

	OptionsMenu->Open();
}

void ExitOptionsMenu() {
	OptionsMenu->Close();
	MainMenu->Open();
}

void OpenConfirmDeleteSaveMenu() {
	LoadGameMenu->Close();
	ConfirmDeleteSaveMenu->Open();
}

void LoadSettings() {
	string save_data_file_name = "settings_data.txt";
	ifstream f(save_data_file_name.c_str());
	if (f.good()) {
		Json::Value save_data;
		string rawData = "";
		string line;
		ifstream myfile(save_data_file_name, ios::binary);

		if (myfile.is_open()) {
			while (getline(myfile, line)) {
				rawData += line;
			}

			myfile.close();
		}
		else {
			cout << "Unable to open file " << save_data_file_name << "\n";
		}

		Json::Reader reader;
		reader.parse(rawData, save_data);

		Singleton<Settings>::Get()->music_volume = save_data["MusicVolume"].asFloat();
		Singleton<Settings>::Get()->effects_volume = save_data["EffectsVolume"].asFloat();
		Singleton<Settings>::Get()->fullscreen = save_data["Fullscreen"].asBool();
		Singleton<Settings>::Get()->using_arrows_for_movement = save_data["UsingArrowsForMovement"].asBool();
		last_used_save_slot = save_data["LastUsedSaveSlot"].asInt();

		myfile.close();
	}
}

void SaveSettings() {
	ofstream ofs("settings_data.txt", ios::binary | ios::out);

	Json::Value save_data;

	save_data["MusicVolume"] = Singleton<Settings>::Get()->music_volume;
	save_data["EffectsVolume"] = Singleton<Settings>::Get()->effects_volume;
	save_data["Fullscreen"] = Singleton<Settings>::Get()->fullscreen;
	save_data["UsingArrowsForMovement"] = Singleton<Settings>::Get()->using_arrows_for_movement;
	save_data["LastUsedSaveSlot"] = last_used_save_slot;

	Json::StreamWriterBuilder wbuilder;
	std::string document = Json::writeString(wbuilder, save_data);

	ofs << document;

	ofs.close();
}

void EnableUsingArrowsForMovement() {
	Singleton<Settings>::Get()->using_arrows_for_movement = true;
}

void DisableUsingArrowsForMovement() {
	Singleton<Settings>::Get()->using_arrows_for_movement = false;
}

void EnableFullscreen() {
	window->setActive(false);

	Singleton<Settings>::Get()->fullscreen = true;
	window->create(sf::VideoMode::getFullscreenModes()[0], "Project Boulder", sf::Style::Fullscreen);
	camera->viewport_dimensions = sf::Vector2f((float)window->getViewport(window->getView()).width, (float)window->getViewport(window->getView()).height);

	window->setActive(true);
}

void DisableFullscreen() {
	window->setActive(false);

	Singleton<Settings>::Get()->fullscreen = false;
	window->create(sf::VideoMode::VideoMode((int)window_width, (int)window_height), "Project Boulder");
	camera->viewport_dimensions = sf::Vector2f(viewport_width, viewport_height);

	window->setActive(true);
}

int main()
{
	if (!sf::Shader::isAvailable())
	{
		cout << "Shaders are not supported on this setup.\n";
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

	bool go_to_credits = false;
	
	LoadSettings();

	Singleton<SmashWorld>::Get()->menuMusicFileName = menuMusicFileName;
	Singleton<SmashWorld>::Get()->combatMusicFileName = combatMusicFileName;
	Singleton<SmashWorld>::Get()->travelingMusicFileName = travelingMusicFileName;
	Singleton<SmashWorld>::Get()->downTimeMusicFileName = downTimeMusicFileName;

	menuMusic = Singleton<SmashWorld>::Get()->GetMusicManager()->AddSong(menuMusicFileName);
	Singleton<SmashWorld>::Get()->GetMusicManager()->AddSong(combatMusicFileName);
	Singleton<SmashWorld>::Get()->GetMusicManager()->AddSong(travelingMusicFileName);
	Singleton<SmashWorld>::Get()->GetMusicManager()->AddSong(downTimeMusicFileName);

	//menuMusic = new sf::Music();
	//if (!menuMusic->openFromFile("Sound/MenuMusic.wav"))
	//	return -1;
	//menuMusic->setVolume(Singleton<Settings>::Get()->music_volume / 2.0f);
	//menuMusic->play();
	//menuMusic->setLoop(true);
	//
	//combatMusic = new sf::Music();
	//if (!combatMusic->openFromFile("Sound/CombatMusic.wav"))
	//	return -1;
	//combatMusic->setVolume(Singleton<Settings>::Get()->music_volume);
	//combatMusic->setLoop(true);
	//
	//travelingMusic = new sf::Music();
	//if (!travelingMusic->openFromFile("Sound/SkateBoardingMusic.wav"))
	//	return -1;
	//travelingMusic->setVolume(0.0f);
	//travelingMusic->play();
	//travelingMusic->setLoop(true);
	//
	//downTimeMusic = new sf::Music();
	//if (!downTimeMusic->openFromFile("Sound/DownTimeMusic.wav"))
	//	return -1;
	//downTimeMusic->setVolume(0.0f);
	//downTimeMusic->play();
	//downTimeMusic->setLoop(true);

	camera = new Camera(sf::Vector2f(0, 0), sf::Vector2f(viewport_width, viewport_height));
	if (Singleton<Settings>::Get()->fullscreen) {
		window = new sf::RenderWindow(sf::VideoMode::getFullscreenModes()[0], "Project Boulder", sf::Style::Fullscreen);
		camera->viewport_dimensions = sf::Vector2f((float)window->getViewport(window->getView()).width, (float)window->getViewport(window->getView()).height);
	} else {
		window = new sf::RenderWindow(sf::VideoMode::VideoMode((int)window_width, (int)window_height), "Project Boulder");// , sf::Style::Fullscreen);
	}
	window->setVerticalSyncEnabled(false);
	//sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode::getDesktopMode(), "Project Boulder");// , sf::Style::Fullscreen);
	//main_character = new PlayerCharacter(window, sf::Vector2f(0.0f, 0.0f), sf::Vector2f(40.0f, 80.0f), true);
	//input_handler = new InputHandler(main_character);

	if (!logo_screen_texture.loadFromFile("Images/LogoScreen.png"))
		return -1;

	logo_screen_sprite = sf::Sprite(logo_screen_texture);
	logo_screen_sprite.setScale(camera->viewport_dimensions.x / logo_screen_texture.getSize().x, camera->viewport_dimensions.y / logo_screen_texture.getSize().y);
	logo_screen_sprite_transparency = 0;

	if (!start_menu_background_texture.loadFromFile("Images/MainMenuBackground.jpg"))
		return -1;

	start_menu_background_sprite = sf::Sprite(start_menu_background_texture);

	if (!ringbearer_font.loadFromFile("Images/RingbearerFont.ttf"))
		return -1;

	title_text = sf::Text("Project Boulder", ringbearer_font, 90);
	title_text.setPosition(viewport_width / 2.0f - 260.0f, viewport_height / 2.0f - 0.0f);

	start_text = sf::Text("Press A/Start to Begin", ringbearer_font);
	start_text.setPosition(viewport_width / 2.0f - 120.0f, viewport_height / 2.0f - 60.0f);

	how_to_delete_save_text = sf::Text("Press Y to Delete Current Selection", ringbearer_font, 25);
	how_to_delete_save_text.setPosition(100.0f, 10.0f);
	how_to_delete_save_text.setFillColor(sf::Color::Red);

	credits_text = sf::Text("Made by Ian James\n\n\n\n\nThank you for playing Project Boulder", ringbearer_font);
	credits_text.setPosition(viewport_width / 2.0f - 220.0f, viewport_height + 50.0f);

	story_text = sf::Text("Oh, yeah, no, that was a bad thing you just did...\n\n\n\n\nThat experiment went real bad.", ringbearer_font);
	story_text.setPosition(viewport_width / 2.0f - 220.0f, viewport_height + 50.0f);

	BuildLoadMenu();
	BuildMainMenu();

	OptionsMenu = new Menu(window, camera->viewport_dimensions);
	OptionsMenu->AddItem("Music Volume", (int)Singleton<Settings>::Get()->music_volume, 100);
	OptionsMenu->AddItem("Effects Volume", (int)Singleton<Settings>::Get()->effects_volume, 100);
	OptionsMenu->AddItem("Fullscreen", Singleton<Settings>::Get()->fullscreen, &EnableFullscreen, &DisableFullscreen);
	OptionsMenu->AddItem("Use Arrows For Movement", Singleton<Settings>::Get()->using_arrows_for_movement, &EnableUsingArrowsForMovement, &DisableUsingArrowsForMovement);
	OptionsMenu->AddItem("Save Settings", &SaveSettings);
	OptionsMenu->AddItem("Back", &ExitOptionsMenu);

	ConfirmDeleteSaveMenu = new Menu(window, camera->viewport_dimensions);
	ConfirmDeleteSaveMenu->AddItem("Do you wish to delete this save, permanently?", &Nothing);
	ConfirmDeleteSaveMenu->AddItem("Yes", &DeleteSave);
	ConfirmDeleteSaveMenu->AddItem("No", &ExitConfirmDeleteSaveMenu);

	soulOriginPoint = sf::Vector2f(viewport_width - 478.0f, viewport_height);
	for (int i = 0; i < numberOfSouls; i++) {
		souls.push_back(new sf::CircleShape(2.0f));
		soulAlphas.push_back((float)(rand() % 100));
		souls[i]->setFillColor(sf::Color(189, 243, 227, (int)soulAlphas[i]));
		souls[i]->setPosition(soulOriginPoint);
		soulVectors.push_back(new sf::Vector2f(0.0f + ((float)((rand() % 100) - 50) / 1000.0f), -0.51f + ((float)((rand() % 100) - 50) / 1000.0f)));
	}

	//Singleton<SmashWorld>::Get()->MenuMusic = menuMusic;
	//Singleton<SmashWorld>::Get()->CombatMusic = combatMusic;
	//Singleton<SmashWorld>::Get()->DownTimeMusic = downTimeMusic;
	//Singleton<SmashWorld>::Get()->TravelingMusic = travelingMusic;

	while (window->isOpen())
	{
		time = clock.getElapsedTime();
		time_current = time.asMicroseconds();

		a_button_current = sf::Joystick::isButtonPressed(0, a_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
		b_button_current = sf::Joystick::isButtonPressed(0, b_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
		y_button_current = sf::Joystick::isButtonPressed(0, y_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
		start_button_current = sf::Joystick::isButtonPressed(0, start_button) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return);

		left_stick_vertical = 0.0f;
		left_stick_horizontal = 0.0f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			left_stick_vertical = -100.0f;
		} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			left_stick_vertical = 100.0f;
		} else {
			left_stick_vertical = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			left_stick_horizontal = 100.0f;
		} else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			left_stick_horizontal = -100.0f;
		} else {
			left_stick_horizontal = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
		}

		if (time_previous + microseconds_per_frame < time_current) {
			frame_delta = time_current - time_previous;
			time_previous = time_current;

			if (GameState == GAME_STATE_LOGOS) {
				UpdateGameStateLogos();
			} else if (GameState == GAME_STATE_START_MENU) {
				UpdateGameStateStartMenu();
			} else if (GameState == GAME_STATE_NEW_SINGLE_PLAYER) {
				UpdateGameStateNewSinglePlayer();
			} else if (GameState == GAME_STATE_INIT_SINGLE_PLAYER) {
				window->clear();
				UpdateGameStateLoadingScreen(); 
				Singleton<SmashWorld>::Get()->Init(window, camera, (float)frames_per_second, selected_save_slot, load_game);
				last_used_save_slot = selected_save_slot;
				SaveSettings();
				if (playGameWithCommentary) {
					Singleton<SmashWorld>::Get()->StartAudioCommentary();
				}
				GameState = GAME_STATE_IN_SINGLE_PLAYER;
			} else if (GameState == GAME_STATE_IN_SINGLE_PLAYER) {
				go_to_credits = Singleton<SmashWorld>::Get()->Update(current_frame, frame_delta / 1000);

				if (go_to_credits) {
					credits_text.setPosition(viewport_width / 2.0f - 220.0f, viewport_height + 50.0f);
					GameState = GAME_STATE_CREDITS;
				} else if (Singleton<SmashWorld>::Get()->ShouldExitToMainMenu()) {
					BuildLoadMenu();
					BuildMainMenu();
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
		Singleton<SmashWorld>::Get()->GetMusicManager()->RestartSong(menuMusicFileName);
		Singleton<SmashWorld>::Get()->GetMusicManager()->FadeToSong(menuMusicFileName);
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
	int return_value = 0;
	bool apply_return_value = false;

	window->clear();
	window->draw(start_menu_background_sprite);

	for (int i = 0; i < numberOfSouls; i++) {
		soulAlphas[i] -= 0.25f;

		if (soulAlphas[i] <= 0) {
			soulAlphas[i] = (float)(rand() % 255);
			souls[i]->setPosition(soulOriginPoint);
			soulVectors[i] = new sf::Vector2f(0.0f + ((float)((rand() % 100) - 50) / 1000.0f), -0.51f + ((float)((rand() % 100) - 50) / 1000.0f));
		}

		souls[i]->setFillColor(sf::Color(189, 243, 227, (int)soulAlphas[i]));

		sf::Vector2f old_position = souls[i]->getPosition(); 
		soulVectors[i]->x = soulVectors[i]->x + ((float)((rand() % 100) - 50) / 10000.0f);
		souls[i]->setPosition(old_position.x + soulVectors[i]->x, old_position.y + soulVectors[i]->y);

		window->draw(*souls[i]);
	}

	if (menuMusic->getVolume() != Singleton<Settings>::Get()->music_volume) {
		menuMusic->setVolume(Singleton<Settings>::Get()->music_volume);
	}

	window->draw(title_text);

	if (MainMenu->IsOpen) {
		if (left_stick_vertical > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			MainMenu->MoveCursorDown();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			MainMenu->MoveCursorUp();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical >= -90.0f && left_stick_vertical <= 90.0f) {
			can_take_another_left_stick_input_from_menu_controller = true;
		}

		MainMenu->Draw(current_frame);
	} else if (OptionsMenu->IsOpen) {
		if (left_stick_vertical > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			OptionsMenu->MoveCursorDown();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			OptionsMenu->MoveCursorUp();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical >= -90.0f && left_stick_vertical <= 90.0f) {
			can_take_another_left_stick_input_from_menu_controller = true;
		}

		if (left_stick_horizontal > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			return_value = OptionsMenu->MoveCursorRight();
			apply_return_value = true;
		} else if (left_stick_horizontal < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			return_value = OptionsMenu->MoveCursorLeft();
			apply_return_value = true;
		}

		if (apply_return_value) {
			if (OptionsMenu->GetCurrentSelectionText() == "Music Volume") {
				Singleton<Settings>::Get()->music_volume = (float)return_value;
			} else if (OptionsMenu->GetCurrentSelectionText() == "Effects Volume") {
				Singleton<Settings>::Get()->effects_volume = (float)return_value;
			}
		}

		OptionsMenu->Draw(current_frame);
	} else if(LoadGameMenu->IsOpen) {
		if (left_stick_vertical > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			LoadGameMenu->MoveCursorDown();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			LoadGameMenu->MoveCursorUp();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical >= -90.0f && left_stick_vertical <= 90.0f) {
			can_take_another_left_stick_input_from_menu_controller = true;
		}

		window->draw(how_to_delete_save_text);
		LoadGameMenu->Draw(current_frame);
	} else if (ConfirmDeleteSaveMenu->IsOpen) {
		if (left_stick_vertical > 90.0f && can_take_another_left_stick_input_from_menu_controller) {
			ConfirmDeleteSaveMenu->MoveCursorDown();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical < -90.0f && can_take_another_left_stick_input_from_menu_controller) {
			ConfirmDeleteSaveMenu->MoveCursorUp();
			can_take_another_left_stick_input_from_menu_controller = false;
		} else if (left_stick_vertical >= -90.0f && left_stick_vertical <= 90.0f) {
			can_take_another_left_stick_input_from_menu_controller = true;
		}

		ConfirmDeleteSaveMenu->Draw(current_frame);
	}

	if (WasButtonAPressed() || WasButtonStartPressed()) {
		if (MainMenu->IsOpen) {
			MainMenu->ExecuteCurrentSelection();
		} else if (OptionsMenu->IsOpen) {
			OptionsMenu->ExecuteCurrentSelection();
		} else if (LoadGameMenu->IsOpen) {
			LoadGameMenu->ExecuteCurrentSelection();
		} else if (ConfirmDeleteSaveMenu->IsOpen) {
			ConfirmDeleteSaveMenu->ExecuteCurrentSelection();
		}
	}

	if (WasButtonBPressed()) {
		if (MainMenu->IsOpen) {
			logo_screen_sprite_transparency = 0;
			GameState = GAME_STATE_LOGOS;
		} else if (OptionsMenu->IsOpen) {
			ExitOptionsMenu();
		} else if (LoadGameMenu->IsOpen) {
			ExitLoadGameMenu();
		} else if (ConfirmDeleteSaveMenu->IsOpen) {
			ExitConfirmDeleteSaveMenu();
		}
	}

	if (LoadGameMenu->IsOpen && WasButtonYPressed()) {
		selected_save_slot = LoadGameMenu->GetCurrentSelectionIndex();
		OpenConfirmDeleteSaveMenu();
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

void UpdateGameStateNewSinglePlayer() {
	window->clear();

	story_text.setPosition(story_text.getPosition().x, story_text.getPosition().y - 1.0f);
	window->draw(story_text);

	if (story_text.getPosition().y < -350.0f || WasButtonStartPressed()) {
		GameState = GAME_STATE_INIT_SINGLE_PLAYER;
	}

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
		BuildLoadMenu();
		GameState = GAME_STATE_START_MENU;
	}

	HandleClosingEvent();
	SetPreviousButtonValues();

	window->display();
}

int IncrementTransparency(int transparency) {
	if (transparency >= 255) {
		transparency = 255;
	} else {
		transparency++;
	}

	return transparency;
}

void SetPreviousButtonValues() {
	a_button_previous = a_button_current;
	b_button_previous = b_button_current;
	y_button_previous = y_button_current;
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

bool WasButtonYPressed() {
	return y_button_current && !y_button_previous;
}

bool WasButtonYReleased() {
	return !y_button_current && y_button_previous;
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

void BuildMainMenu() {
	MainMenu = new Menu(window, camera->viewport_dimensions);

	if (last_used_save_slot != 99) {
		MainMenu->AddItem("Continue", &LoadGameOfLastUsedSavedSlot);
	}

	if (first_empty_save_slot_found == -1) {
		MainMenu->AddItem("New Game (All Save Slots are Full)", &NewGame);
		MainMenu->SetEnabled("New Game (All Save Slots are Full)", false);
	} else {
		MainMenu->AddItem("New Game", &NewGame);
	}
	//MainMenu->AddItem("Play Game With Audio Commentary", &NewGameWithCommentary);
	MainMenu->AddItem("Load Saved Game", &OpenLoadGameMenu);
	MainMenu->AddItem("Options", &OpenOptionsMenu);
	MainMenu->AddItem("Exit", &Exit);
}

void BuildLoadMenu() {
	LoadGameMenu = new Menu(window, camera->viewport_dimensions);
	int first_empty_save_slot_found_this_time = -1;

	for (int i = 0; i < maxNumberOfSaveSlots; i++) {
		string save_data_file_name = "save" + to_string(i) + ".sav";
		ifstream f(save_data_file_name.c_str());
		if (f.good()) {
			string stage_of_game = "";
			string character_level = "";

			ExtractSavedGameDisplayInformation(save_data_file_name, &stage_of_game, &character_level);

			string display = "Save #" + to_string(i + 1) + " | Stage of Game: ";
			display.append(stage_of_game);
			display.append(" | Character Level: ");
			display.append(character_level);

			callback_function_with_param fn_load_game = &LoadGame;
			LoadGameMenu->AddItem(display, *fn_load_game, i);
			//LoadGameMenu->AddItem(*stage_of_game + " " + *character_level, std::bind(LoadGame, save_data_file_name));
		} else {
			if (first_empty_save_slot_found_this_time == -1) {
				first_empty_save_slot_found_this_time = i;
			}
			LoadGameMenu->AddItem("Empty Slot", &Nothing);
		}
	}

	first_empty_save_slot_found = first_empty_save_slot_found_this_time;
}

void ExtractSavedGameDisplayInformation(string file_path, string* stage_of_game, string* character_level) {
	Json::Value save_data;
	string rawData = "";
	string line;
	ifstream myfile(file_path, ios::binary);

	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			rawData += line;
		}

		myfile.close();
	}
	else {
		cout << "Unable to open file " << file_path << "\n";
	}

	Json::Reader reader;
	reader.parse(rawData, save_data);

	*stage_of_game = save_data["WorldData"]["StageOfTheGame"].asString();
	*character_level = to_string(save_data["Player"]["CharacterLevel"].asInt());
}