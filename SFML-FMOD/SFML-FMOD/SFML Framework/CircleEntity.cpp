#include "CircleEntity.h"

CircleEntity::CircleEntity(sf::Vector2f p_position, float p_radius)
{
	setRadius(p_radius);
	setPosition(p_position);

	collider.setRadius(p_radius);
	collider.setPosition(sf::Vector2f(getPosition().x + getRadius(), getPosition().y + getRadius()));
}

CircleEntity::CircleEntity()
{
	
}

CircleEntity::~CircleEntity()
{
}

void CircleEntity::fixedUpdate(float dt)
{
	CircleShape::update();
}

void CircleEntity::variableUpdate(float dt)
{
}

CircleCollider* CircleEntity::getCollider()
{
	collider.setRadius(getRadius());

	collider.setPosition(sf::Vector2f(getPosition().x + getRadius(), getPosition().y + getRadius()));
	return & collider;
}
