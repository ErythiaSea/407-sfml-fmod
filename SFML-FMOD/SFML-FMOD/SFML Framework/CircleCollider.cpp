#include "CircleCollider.h"

CircleCollider::CircleCollider(sf::Vector2f p_centre, float p_radius)
{
	position = p_centre;
	radius = p_radius;
}

CircleCollider::CircleCollider()
{
}

CircleCollider::~CircleCollider()
{
	
}


float CircleCollider::getRadius()
{
	return radius;
}

void CircleCollider::setRadius(float new_radius)
{
	radius = new_radius;
}
