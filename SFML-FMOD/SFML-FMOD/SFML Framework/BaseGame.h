#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

#include "Input.h"
#include "Entity.h"
#include "CollisionEntity.h"
#include "CircleEntity.h"
#include "AudioManager.h"
#include "Animation.h"
#include "Collision.h"

class BaseGame
{
public:
	BaseGame() {};
	BaseGame(sf::RenderWindow* window);
	~BaseGame();

	virtual void handleInput(Input *in);

	virtual void fixedUpdate(float fixed_timestep);
	virtual void networkUpdate(float fixed_timestep); // todo; variable timestep
	virtual void handleCollisions(float fixed_timestep);

	virtual void variableUpdate(float variable_timestep);
	virtual void renderGame();

protected:
	sf::RenderWindow* game_window;
};

