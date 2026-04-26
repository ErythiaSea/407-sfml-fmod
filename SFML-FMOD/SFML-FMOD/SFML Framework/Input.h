#include "SFML/Graphics.hpp"
#include <iostream>
#include <map>

#pragma once
class Input
{
public:

	void setControllerButtonDown(int input_id, bool state);
	void setJoystickAxis(sf::Joystick::Axis axis, float value);
	void setKeyDown(sf::Keyboard::Key key, bool state);
	void setMouseButtonDown(sf::Mouse::Button mb, bool state);
	void setMousePosition(sf::RenderWindow* wn);

	// Used to determine if a key is currently being held down.
	bool isKeyDown(sf::Keyboard::Key key) const;

	// Used to determine if a key was just pressed.
	bool isKeyPressed(sf::Keyboard::Key key);

	// Used to determine if a key was just released.
	bool isKeyReleased(sf::Keyboard::Key key);

	// Used to determine if a mouse button is currently being held down.
	bool isMouseButtonDown(sf::Mouse::Button mb) const;

	// Used to determine if a mouse button was just pressed.
	bool isMouseButtonPressed(sf::Mouse::Button mb);

	// Used to determine if a mouse button was just released.
	bool isMouseButtonReleased(sf::Mouse::Button mb);

	sf::Vector2f getMousePosWorld(sf::RenderWindow* wn = nullptr) const;

	// Clears the pressed/released map for next tick.
	void update();

	bool a_button_down = false;
	bool b_button_down = false;
	bool x_button_down = false;
	bool y_button_down = false;

	bool left_bumper_down = false;
	bool right_bumper_down = false;

	bool select_button_down = false;
	bool start_button_down = false;

	bool lstick_button_down = false;
	bool rstick_button_down = false;

	bool left_trigger = false;
	bool right_trigger = false;

	bool dpad_down = false;
	bool dpad_up = false;
	bool dpad_left = false;
	bool dpad_right = false;

	sf::Vector2f left_joystick = {0,0};
	sf::Vector2f right_joystick = {0,0};

	sf::Vector2i mouse_pos_local = { 0,0 }; // position of mouse within the window
	sf::Vector2i mouse_pos_global = { 0,0 }; // position of mouse on screen

	// Set a default render window for functions that require one.
	void setDefaultWindow(sf::RenderWindow* wn);

protected:
	float dpad_tolerance = 50;	
	float trigger_tolerance = 50;

private:
	bool keys[sf::Keyboard::KeyCount]{ false };
	std::map<sf::Keyboard::Key, bool> recently_updated_keys;

	bool mouse_buttons[sf::Mouse::ButtonCount]{ false };
	std::map<sf::Mouse::Button, bool> recently_updated_mouse_buttons;

	sf::RenderWindow* defaultWn = nullptr;
};

