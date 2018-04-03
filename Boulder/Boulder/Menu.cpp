#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Menu.h"
#include "SmashWorld.h"
#include <fstream>

Menu::Menu(sf::RenderWindow* window, sf::Vector2f dimensions, string background_file_name) {
	render_window = window;
	viewport_dimensions = dimensions;
	cursor_position = 0;
	cursor = sf::RectangleShape(sf::Vector2f(10.0f, 10.0f));
	IsOpen = false;

	if (background_file_name != "") {
		BackgroundTexture = Singleton<AssetManager>().Get()->GetTexture(background_file_name);
		BackgroundSprite = new sf::Sprite(*BackgroundTexture);
	}
}

void Menu::Open() {
	cursor_position = 0;
	IsOpen = true;
}

void Menu::Close() {
	IsOpen = false;
}

void Menu::ExecuteCurrentSelection() {
	menu_items[cursor_position].ExecutionAction();
}

void Menu::AddItem(string text, callback_function pFunc) {
	menu_items.push_back(MenuItem(text, pFunc));

	int item_in_list = (int)menu_items.size() - 1;
}

void Menu::AddItem(string text, int current_value, int number_of_possible_values) {
	menu_items.push_back(MenuItem(text, current_value, number_of_possible_values));

	int item_in_list = (int)menu_items.size() - 1;
}

void Menu::AddItem(string text, bool checked, callback_function on_checked_action, callback_function on_unchecked_action) {
	menu_items.push_back(MenuItem(text, checked, on_checked_action, on_unchecked_action));

	int item_in_list = (int)menu_items.size() - 1;
}

void Menu::Draw(sf::Int64 curr_frame) {
	if (BackgroundSprite != nullptr) {
		render_window->draw(*BackgroundSprite);
	}

	for (int i = 0; i < (int)menu_items.size(); i++) {
		menu_items[i].Draw(render_window, sf::Vector2f(50.0f, 50.0f + 50.0f * i));
	}

	current_y_offset += (int)((curr_frame % 61) - 30) / 120.0f;

	sf::Vector2f menu_item_position = menu_items[cursor_position].GetMenuItemText().getPosition();
	cursor.setPosition(sf::Vector2f(menu_item_position.x - 20.0f, menu_item_position.y + 25.0f + current_y_offset));
	render_window->draw(cursor);
}

void Menu::MoveCursorDown() {
	cursor_position++;

	if (cursor_position >= (int)menu_items.size()) {
		cursor_position = 0;
	}
}

void Menu::MoveCursorUp() {
	cursor_position--;

	if (cursor_position < 0) {
		cursor_position = (int)menu_items.size() - 1;
	}
}

int Menu::MoveCursorRight() {
	return menu_items[cursor_position].MoveSliderRight();
}

int Menu::MoveCursorLeft() {
	return menu_items[cursor_position].MoveSliderLeft();
}

string Menu::GetCurrentSelectionText() {
	return menu_items[cursor_position].GetString();
}

void Menu::SetCurrentSliderValueByText(string menu_item_text, int new_value) {
	int menu_items_size = (int)menu_items.size();
	for (int i = 0; i < menu_items_size; i++) {
		if (menu_items[i].GetString() == menu_item_text) {
			menu_items[i].SetCurrentSliderValue(new_value);
		}
	}
}

int Menu::GetCurrentSliderValueByText(string menu_item_text) {
	int menu_items_size = (int)menu_items.size();
	for (int i = 0; i < menu_items_size; i++) {
		if (menu_items[i].GetString() == menu_item_text) {
			return menu_items[i].GetCurrentSliderValue();
		}
	}

	return 0;
}

MenuItem::MenuItem(string text, callback_function action) {
	Text = text;
	Action = action;
	Font.loadFromFile("Images/RingbearerFont.ttf");
	MenuItemText = sf::Text(text, Font, 45);

	CheckBox = false;
	Checked = false;
}

MenuItem::MenuItem(string text, int current_value, int number_of_possible_values) {
	Text = text;
	CurrentSliderValue = current_value;
	TotalSliderValues = number_of_possible_values;

	SliderLine = new sf::RectangleShape(sf::Vector2f(300, 4));
	SliderCursor = new sf::RectangleShape(sf::Vector2f(20, 8));
	SliderCursor->rotate(90);

	Font.loadFromFile("Images/RingbearerFont.ttf");
	MenuItemText = sf::Text(text, Font, 45);

	CheckBox = false;
	Checked = false;
}

MenuItem::MenuItem(string text, bool checked, callback_function on_checked_action, callback_function on_unchecked_action) {
	Text = text;
	Checked = checked;
	Action = nullptr;
	Font.loadFromFile("Images/RingbearerFont.ttf");
	MenuItemText = sf::Text(text, Font, 45);

	CheckBox = true;

	CheckBoxRect = new sf::RectangleShape(sf::Vector2f(30, 30));
	CheckBoxRect->setFillColor(sf::Color::Black);
	CheckBoxRect->setOutlineThickness(3.0f);
	CheckBoxRect->setOutlineColor(sf::Color::White);

	CheckBoxCheckedRect = new sf::RectangleShape(sf::Vector2f(20, 20));
	CheckBoxCheckedRect->setFillColor(sf::Color::White);

	OnChecked = on_checked_action;
	OnUnchecked = on_unchecked_action;
}

string MenuItem::GetString() {
	return Text;
}

sf::Text MenuItem::GetMenuItemText() {
	return MenuItemText;
}

void MenuItem::ExecutionAction() {
	if (Enabled()) {
		if (CheckBox) {
			Checked = !Checked;

			if (Checked) {
				OnChecked();
			} else {
				OnUnchecked();
			}
		} else {
			Action();
		}
	}
}

int MenuItem::MoveSliderRight() {
	if (TotalSliderValues == 0) {
		return 0;
	}

	CurrentSliderValue++;

	if (CurrentSliderValue > TotalSliderValues) {
		CurrentSliderValue = TotalSliderValues;
	}

	return CurrentSliderValue;
}

int MenuItem::MoveSliderLeft() {
	if (TotalSliderValues == 0) {
		return 0;
	}

	CurrentSliderValue--;

	if (CurrentSliderValue < 0) {
		CurrentSliderValue = 0;
	}

	return CurrentSliderValue;
}

void MenuItem::Draw(sf::RenderWindow* window, sf::Vector2f position) {
	MenuItemText = sf::Text(Text, Font, 45);
	MenuItemText.setPosition(position);

	sf::Color gray = sf::Color(105, 105, 105);

	if (!Enabled()) {
		MenuItemText.setFillColor(gray);
		MenuItemText.setOutlineColor(gray);
	}

	if (TotalSliderValues != 0) {
		SliderLine->setPosition(sf::Vector2f(position.x + 395.0f, position.y + 23.0f));
		SliderCursor->setPosition(sf::Vector2f(position.x + 400.0f + ((300.0f / TotalSliderValues) * (CurrentSliderValue - 1)), position.y + 16.0f));

		window->draw(*SliderLine);
		window->draw(*SliderCursor);
	}

	if (CheckBox) {
		CheckBoxRect->setPosition(sf::Vector2f(position.x + 395.0f, position.y + 15.0f));
		window->draw(*CheckBoxRect);

		if (Checked) {
			CheckBoxCheckedRect->setPosition(sf::Vector2f(position.x + 400.0f, position.y + 20.0f));
			window->draw(*CheckBoxCheckedRect);
		}
	}

	window->draw(MenuItemText);
}

bool MenuItem::Enabled() {
	if (Text == "Load Game") {
		string save_data_file_name = "save_data.txt";
		ifstream f(save_data_file_name.c_str());
		if (!f.good()) {
			return false;
		}
	}

	return true;
}

void MenuItem::SetCurrentSliderValue(int new_value) {
	CurrentSliderValue = new_value;
}

int MenuItem::GetCurrentSliderValue() {
	return CurrentSliderValue;
}