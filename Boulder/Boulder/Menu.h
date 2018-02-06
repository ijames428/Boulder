#ifndef MENU_H
#define MENU_H

using namespace std;
#include <SFML/Graphics.hpp>

typedef void(*callback_function)(void); // type for conciseness

class MenuItem {
private:
	typedef void(*callback_function)(void); // type for conciseness
	sf::Text MenuItemText;
	string Text;
	callback_function Action;
	sf::Font Font;
public:
	MenuItem(string text, callback_function pFunc);
	string GetText();
	void ExecutionAction();
	sf::Text GetMenuItemText();
	void Draw(sf::RenderWindow* window, sf::Vector2f position);
	bool Enabled();
};

class Menu {
public:
	Menu(sf::RenderWindow* window, sf::Vector2f dimensions);
	void AddItem(string text, callback_function pFunc);
	void Draw(sf::Int64 curr_frame);
	void MoveCursorDown();
	void MoveCursorUp();
	void Open();
	void Close();
	bool IsOpen;
	void ExecuteCurrentSelection();
private:
	sf::RectangleShape cursor;
	int cursor_position;
	std::vector<MenuItem> menu_items;
	sf::RenderWindow* render_window;
	sf::Vector2f viewport_dimensions;
	sf::Font ringbearer_font;
	float current_y_offset;
};

#endif