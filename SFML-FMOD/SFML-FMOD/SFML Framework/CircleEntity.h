#pragma once
#include "CircleCollider.h"
#include "SFML/Graphics.hpp"

class CircleEntity :public sf::CircleShape
{
public:
	CircleEntity(sf::Vector2f p_position,float p_radius);
	CircleEntity();
	~CircleEntity();

	void fixedUpdate(float dt);
	void variableUpdate(float dt);

	CircleCollider* getCollider();

private:
	CircleCollider collider;
};

