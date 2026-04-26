#include "ColliderShape.h"


ColliderShape::ColliderShape(sf::Vector2f p_centre, sf::Vector2f p_size) //square colliders
{
	custom = false;

	centre = p_centre;
	size = p_size;

	setCorners();
}

ColliderShape::ColliderShape(sf::Vector2f p_centre, std::array<sf::Vector2f, 4> p_corners) //non-square colliders
{
	custom = true;

	corners = p_corners;
}

ColliderShape::ColliderShape()
{
}

ColliderShape::~ColliderShape()
{
}

void ColliderShape::update(sf::Transform p_transform_to_apply)
{
	transformed_corners = {
	p_transform_to_apply.transformPoint(corners[0]),
	p_transform_to_apply.transformPoint(corners[1]),
	p_transform_to_apply.transformPoint(corners[2]),
	p_transform_to_apply.transformPoint(corners[3])
	};
}

void ColliderShape::setCentre(sf::Vector2f p_new_centre)
{
	centre = p_new_centre;

	setCorners();
}

void ColliderShape::setSize(sf::Vector2f p_new_size)
{
	size = p_new_size;

	setCorners();
}

std::array<sf::Vector2f, 4> ColliderShape::getTransformedCorners()
{

	return transformed_corners;
}

std::array<sf::Vector2f, 4> ColliderShape::getLocalCorners()
{
	return corners;
}


std::array<sf::Vector2f, 4> ColliderShape::getEdges()
{
	return std::array<sf::Vector2f, 4> {
		transformed_corners[1] - transformed_corners[0],
		transformed_corners[2] - transformed_corners[1],
		transformed_corners[3] - transformed_corners[2],
		transformed_corners[0] - transformed_corners[3]
	};
}

sf::FloatRect ColliderShape::getFloatRect()
{
	return sf::FloatRect(position, size);
}

void ColliderShape::setCorners()
{
	if (!custom)
	{
		corners = {
			sf::Vector2f(centre.x - (size.x / 2),centre.y - (size.y / 2)), // top left
			sf::Vector2f(centre.x + (size.x / 2),centre.y - (size.y / 2)) , // top right
			sf::Vector2f(centre.x + (size.x / 2),centre.y + (size.y / 2)) , // bottom right
			sf::Vector2f(centre.x - (size.x / 2),centre.y + (size.y / 2))   // bottom left
		};
	}
}


