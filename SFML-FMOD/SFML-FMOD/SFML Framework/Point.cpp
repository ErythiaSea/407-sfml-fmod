#include "Point.h"

Point::Point(sf::Vector2f p_position)
{
	position = p_position;
}

Point::Point()
{
}

Point::~Point()
{
}

sf::Vector2f Point::getPosition()
{
	return position;
}

void Point::setPosition(sf::Vector2f new_position)
{
	position = new_position;
}