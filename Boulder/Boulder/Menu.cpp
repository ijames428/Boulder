#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Menu.h"
#include "SmashWorld.h"

Menu::Menu(sf::RenderWindow* window, sf::Vector2f dimensions) {
	render_window = window;
	viewport_dimensions = dimensions;
	cursor_position = 0;
	cursor = sf::RectangleShape(sf::Vector2f(10.0f, 10.0f));
	IsOpen = false;
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
	//menu_items[item_in_list].GetMenuItemText().setPosition(sf::Vector2f(50.0f, 50.0f + 50.0f * item_in_list));
}

void Menu::Draw(sf::Int64 curr_frame) {
	for (int i = 0; i < (int)menu_items.size(); i++) {
		menu_items[i].Draw(render_window, sf::Vector2f(50.0f, 50.0f + 50.0f * i));
	}

	current_y_offset += (int)((curr_frame % 60) - 30) / 120.0f;

	cursor.setPosition(sf::Vector2f(menu_items[cursor_position].GetMenuItemText().getPosition().x - 20.0f, menu_items[cursor_position].GetMenuItemText().getPosition().y + 25.0f + current_y_offset));
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

MenuItem::MenuItem(string text, callback_function action) {
	Text = text;
	Action = action;
	Font.loadFromFile("Images/RingbearerFont.ttf");
	MenuItemText = sf::Text(text, Font, 45);
}

string MenuItem::GetText() {
	return Text;
}

sf::Text MenuItem::GetMenuItemText() {
	return MenuItemText;
}

void MenuItem::ExecutionAction() {
	Action();
}

void MenuItem::Draw(sf::RenderWindow* window, sf::Vector2f position) {
	MenuItemText = sf::Text(Text, Font, 45);
	MenuItemText.setPosition(position);
	window->draw(MenuItemText);
}