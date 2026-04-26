#pragma once

#include <SFML/Graphics.hpp>

class Entity: public sf::RectangleShape
{
public:

	Entity();
	Entity(sf::Vector2f position, sf::Vector2f size);
	~Entity();

	virtual void handleInput(float dt);

	virtual void fixedUpdate(float dt);
	virtual void variableUpdate(float dt);

	sf::FloatRect getCollisionRect();
	void setCollisionRect(sf::FloatRect fr) { collisionRect = fr; }

	// sum of position and geometric center
	sf::Vector2f getWorldCenter() { return getPosition() + getGeometricCenter(); }

	sf::Vector2f velocity = sf::Vector2f(0, 0);
	
	bool hasCollision = false;

protected:
	sf::FloatRect collisionRect;
	sf::Vector2f colOffset;
};

