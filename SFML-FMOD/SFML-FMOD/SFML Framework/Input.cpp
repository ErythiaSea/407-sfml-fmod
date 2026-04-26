#include "Input.h"

void Input::setControllerButtonDown(int input_id, bool state)
{

	switch (input_id)
	{
	case(0):b_button_down = state;
		break;
	case(1):a_button_down = state;
		break;
	case(2):y_button_down = state;
		break;
	case(3):x_button_down = state;
		break;
	case(4):left_bumper_down = state;
		break;
	case(5):right_bumper_down = state;
		break;
	case(6):select_button_down = state;
		break;
	case(7):start_button_down = state;
		break;
	case(8):lstick_button_down = state;
		break;
	case(9):rstick_button_down = state;
		break;
	default:
		break;
	}
}

void Input::setJoystickAxis(sf::Joystick::Axis axis, float value)
{
	switch (axis)
	{
	case(sf::Joystick::Axis::X): //X axis (l stick)
		left_joystick.x = value;
		break;
	case(sf::Joystick::Axis::Y): //Y axis (l stick)
		left_joystick.y = value;
		break;
	case(sf::Joystick::Axis::U): //U axis (r stick)
		right_joystick.x = value;
		break;
	case(sf::Joystick::Axis::V): //V axis (r stick)
		right_joystick.y = value;
		break;
	case(sf::Joystick::Axis::Z): //Z Axis, triggers, r tigger is negative, l trigger is positive // Unfortunatley only supports one trigger being used at a time
		if (value >= trigger_tolerance)
		{
			left_trigger = true;
		}
		else
		{
			left_trigger = false;
		}

		if (value <= -trigger_tolerance)
		{
			right_trigger = true;
		}
		else {
			right_trigger = false;
		}
		break;
	case(sf::Joystick::Axis::PovX):
		if (value >= 50)
		{
			dpad_right = true;
		}
		else
		{
			dpad_right = false;
		}

		if (value <= -50)
		{
			dpad_left = true;
		}
		else
		{
			dpad_left = false;
		}
		break;
	case(sf::Joystick::Axis::PovY):
		if (value >= 50)
		{
			dpad_up = true;
		}
		else
		{
			dpad_up = false;
		}

		if (value <= -50)
		{
			dpad_down = true;
		}
		else
		{
			dpad_down = false;
		}
		break;
	default:
		break;
	}
}

void Input::setKeyDown(sf::Keyboard::Key key, bool state)
{
	int idx = static_cast<int>(key);
	keys[idx] = state;
	recently_updated_keys[key] = state;
}

bool Input::isKeyDown(sf::Keyboard::Key key) const
{
	int idx = static_cast<int>(key);
	return keys[idx];
}

bool Input::isKeyPressed(sf::Keyboard::Key key)
{
	// key can't just be pressed if it isn't down
	if (!isKeyDown(key)) return false;

	// check if the key exists in map before accessing with op[]
	// to prevent inserting that key into map
	if (recently_updated_keys.count(key) == 0) return false;
	return recently_updated_keys[key];
}

bool Input::isKeyReleased(sf::Keyboard::Key key)
{
	// key can't just be released if it is down
	if (isKeyDown(key)) return false;

	// check if the key exists in map before accessing with op[]
	// to prevent inserting that key into map
	if (recently_updated_keys.count(key) == 0) return false;
	return !recently_updated_keys[key];
}

void Input::setMouseButtonDown(sf::Mouse::Button mb, bool state)
{
	int idx = static_cast<int>(mb);
	mouse_buttons[idx] = state;
	recently_updated_mouse_buttons[mb] = state;
}

void Input::setMousePosition(sf::RenderWindow* wn)
{
	mouse_pos_global = sf::Mouse::getPosition();
	mouse_pos_local = sf::Mouse::getPosition(*wn); //pass in the window so that we can find the position of the mouse within it

}

bool Input::isMouseButtonDown(sf::Mouse::Button mb) const
{
	int idx = static_cast<int>(mb);
	return mouse_buttons[idx];
}

bool Input::isMouseButtonPressed(sf::Mouse::Button mb)
{
	// key can't just be pressed if it isn't down
	if (!isMouseButtonDown(mb)) return false;

	// check if the key exists in map before accessing with op[]
	// to prevent inserting that key into map
	if (recently_updated_mouse_buttons.count(mb) == 0) return false;
	return recently_updated_mouse_buttons[mb];
}

bool Input::isMouseButtonReleased(sf::Mouse::Button mb)
{
	// key can't just be released if it is down
	if (isMouseButtonDown(mb)) return false;

	// check if the key exists in map before accessing with op[]
	// to prevent inserting that key into map
	if (recently_updated_mouse_buttons.count(mb) == 0) return false;
	return !recently_updated_mouse_buttons[mb];
}

sf::Vector2f Input::getMousePosWorld(sf::RenderWindow* wn) const
{
	assert(wn != nullptr || defaultWn != nullptr);

	if (wn != nullptr)
		return wn->mapPixelToCoords(mouse_pos_local, wn->getView());

	return defaultWn->mapPixelToCoords(mouse_pos_local, defaultWn->getView());
}

void Input::update()
{
	recently_updated_keys.clear();
	recently_updated_mouse_buttons.clear();
}

void Input::setDefaultWindow(sf::RenderWindow* wn)
{
	defaultWn = wn;
}

