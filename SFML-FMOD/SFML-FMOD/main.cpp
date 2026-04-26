/*	CMP425/501 Assessment Project Template.
* 
*	This is an template project that renders an empty SFML window.
*	Use this as a starting point of your final assessment.
*	There are two build configurations -- client and server. 
*	You can include/exclude source files for each configuration, 
*	if you need to build different versions of your game.
*/

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <sstream>
#include <iomanip>
#include <variant>
#include "utils.h"
#include "Game.h"
#include "NetworkManager.h"

void handleEvents(sf::RenderWindow* wn, Input* in)
{
    while (const std::optional var = wn->pollEvent())
    {
        if (var->is<sf::Event::Closed>()) //on window being closed
        {
            wn->close();
        }

        if (const auto* resized = var->getIf<sf::Event::Resized>()) //on window size being changed
        {
            sf::FloatRect visible_area(sf::Vector2f(0, 0), sf::Vector2f(resized->size.x, resized->size.y));
            wn->setView(sf::View(visible_area));
        }

        if (const auto* joystick_button_pressed = var->getIf<sf::Event::JoystickButtonPressed>()) //gamepad buttons being pressed
        {
            in->setControllerButtonDown(joystick_button_pressed->button, true);
        }

        if (const auto* joystick_button_released = var->getIf<sf::Event::JoystickButtonReleased>()) //gamepad buttons being released
        {
            in->setControllerButtonDown(joystick_button_released->button, false);
        }

        if (const auto* joystickMoved = var->getIf<sf::Event::JoystickMoved>()) //leftstick or rightstick being moved
        {
            in->setJoystickAxis(joystickMoved->axis, joystickMoved->position);
        }

        if (const auto* keyboard_key_pressed = var->getIf<sf::Event::KeyPressed>())
        {
            in->setKeyDown(keyboard_key_pressed->code, true);
        }

        if (const auto* keyboard_key_released = var->getIf<sf::Event::KeyReleased>())
        {
            in->setKeyDown(keyboard_key_released->code, false);
        }

        if (const auto* mouse_button_pressed = var->getIf<sf::Event::MouseButtonPressed>())
        {
            in->setMouseButtonDown(mouse_button_pressed->button, true);
        }

        if (const auto* mouse_button_released = var->getIf<sf::Event::MouseButtonReleased>())
        {
            in->setMouseButtonDown(mouse_button_released->button, false);
        }

        if (const auto* mouse_moved = var->getIf<sf::Event::MouseMoved>()) // whenever the mouse moves this is called
        {
            in->setMousePosition(wn);
        }
    }
}

void loadAudio(AudioManager* am) {
    SoundObject* s;

    s = am->addSound("sfx/jump.wav", "jump");
    s->getSound()->setVolume(5.0f);
    s->setMaxConcurrent(2);
}

int main()
{    
	Utils::printMsg("Game startup...");

    Random::newRandomSeed();

    // Configure networking.
    NetworkManager networkMgr;
    networkMgr.setupNetworking();

	// Prepare window.
    sf::String windowTitle = "CMP425 Assessment - PPP - ";
    windowTitle += (networkMgr.isHost() ? "Host" : ("Player ?"));
	sf::RenderWindow window(sf::VideoMode({ 1080, 720 }), windowTitle);
	window.setFramerateLimit(300);	//Request 60 frames per second
    window.setKeyRepeatEnabled(false); // disable repeat key presses on hold
	Utils::printMsg("Window ready...");

    // disable print spam by default
    Utils::printMsg("Disabling print messages! Re-enable with O. Disable again with X.", warning);
    Utils::togglePrints(false);

	// Clock for timing the 'dt' value.
	sf::Clock clock;

    //AudioManager aud;
    //loadAudio(&aud);

    Input in;
    in.setDefaultWindow(&window);
    Game game(&window);

    if (!networkMgr.isHost()) {
        game.addPlayer(true, networkMgr.getConnectionWithId(0));
    }

    const float fixedTimestep = 1.f / 300.f;
	while (window.isOpen()) {
		// Calculate dt.
		float dt = clock.restart().asSeconds();

        // process sfml events
        handleEvents(&window, &in);

        // update the game and process inputs
        game.handleInput(fixedTimestep, &in);
        game.fixedUpdate(fixedTimestep);
        game.handleCollisions(fixedTimestep);

        // do network processing
        networkMgr.handleDisconnections();
        game.networkUpdate(fixedTimestep, &networkMgr);

        // render entities
        game.renderGame();

        // reset press/release functions for next frame
        in.update();
	}

	return 0;
}

