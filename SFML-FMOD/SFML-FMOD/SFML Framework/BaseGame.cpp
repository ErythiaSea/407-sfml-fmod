#include "BaseGame.h"

BaseGame::BaseGame(sf::RenderWindow* window)
{
	game_window = window;
}

BaseGame::~BaseGame()
{

}

void BaseGame::handleInput(Input *in)
{

}

void BaseGame::fixedUpdate(float fixed_timestep)
{

}

void BaseGame::networkUpdate(float fixed_timestep)
{

}

void BaseGame::handleCollisions(float fixed_timestep)
{

}

void BaseGame::variableUpdate(float variable_timestep)
{

}


void BaseGame::renderGame()
{
    game_window->clear(sf::Color::White);

	game_window->display();
}
