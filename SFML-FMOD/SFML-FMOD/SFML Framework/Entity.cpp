#include "Entity.h"

Entity::Entity()
{
	setSize(sf::Vector2f(10.f,10.f));
	setPosition(sf::Vector2f(0, 0));
	collisionRect.size = sf::Vector2f();
	collisionRect.position = sf::Vector2f();
}

Entity::Entity(sf::Vector2f p_position, sf::Vector2f p_size)
{
	setSize(p_size);
	setPosition(p_position);
}

Entity::~Entity()
{
}

void Entity::handleInput(float dt)
{
}

void Entity::fixedUpdate(float dt)
{
	RectangleShape::update();
	
	RectangleShape::setPosition(sf::Vector2f(getPosition().x + velocity.x, getPosition().y + velocity.y)); //moves the object by volcoty each fixed update
}

void Entity::variableUpdate(float dt)
{
}

sf::FloatRect Entity::getCollisionRect()
{
	sf::FloatRect r = sf::FloatRect(getPosition() + collisionRect.position, collisionRect.size);
	return r;
}

