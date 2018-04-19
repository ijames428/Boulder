#ifndef MENU_H
#define MENU_H

using namespace std;
#include <SFML/Graphics.hpp>

typedef void(*callback_function)(void); // type for conciseness
typedef void(*callback_function_with_param)(int); // type for conciseness

class MenuItem {
private:
	sf::Text MenuItemText;
	string Text;
	callback_function Action;
	callback_function_with_param ActionWithParameter;
	sf::Font Font;
	int CurrentSliderValue;
	int TotalSliderValues = 0;
	sf::RectangleShape* SliderLine;
	sf::RectangleShape* SliderCursor;
	bool CheckBox;
	bool Checked;
	sf::RectangleShape* CheckBoxRect;
	sf::RectangleShape* CheckBoxCheckedRect;
	callback_function OnChecked;
	callback_function OnUnchecked;
	int CallBackIntParam;
	bool UseCallBackWithIntParam;
	bool IsEnabled = true;
public:
	MenuItem(string text, callback_function pFunc);
	MenuItem(string text, callback_function_with_param pFunc, int callback_int_param);
	MenuItem(string text, int current_value, int number_of_possible_values);
	MenuItem(string text, bool checked, callback_function on_checked_action, callback_function on_unchecked_action);
	string GetString();
	void ExecutionAction();
	sf::Text GetMenuItemText();
	void Draw(sf::RenderWindow* window, sf::Vector2f position);
	bool Enabled();
	int MoveSliderRight();
	int MoveSliderLeft();
	void SetCurrentSliderValue(int new_value);
	int GetCurrentSliderValue();
	void SetEnabled(bool enabled);
};

class Menu {
public:
	Menu(sf::RenderWindow* window, sf::Vector2f dimensions, string background_file_name = "");
	void AddItem(string text, callback_function pFunc);
	void AddItem(string text, callback_function_with_param pFunc, int callback_int_param);
	void AddItem(string text, int current_value, int number_of_possible_values);
	void AddItem(string text, bool current_value, callback_function on_checked_action, callback_function on_unchecked_action);
	void Draw(sf::Int64 curr_frame);
	void MoveCursorDown();
	void MoveCursorUp();
	int MoveCursorRight();
	int MoveCursorLeft();
	void Open();
	void Close();
	bool IsOpen;
	void ExecuteCurrentSelection();
	int GetCurrentSelectionIndex();
	string GetCurrentSelectionText();
	void SetCurrentSliderValueByText(string menu_item_text, int new_value);
	int GetCurrentSliderValueByText(string menu_item_text);
	void SetEnabled(string menu_item_text, bool enabled);
	sf::Texture* BackgroundTexture;
	sf::Sprite* BackgroundSprite;
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