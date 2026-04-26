#pragma once

#include "Point.h"

class CircleCollider : public Point
{
public:
	CircleCollider(sf::Vector2f p_centre, float p_radius);
	CircleCollider();
	~CircleCollider();

	float getRadius();
	void setRadius(float new_radius);

private:
	float radius;
};

