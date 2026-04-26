#include "Collision.h"

bool Collision::checkCollision(Point * first, Point * second)
{	
	
	if (ColliderShape* rect = dynamic_cast<ColliderShape*>(first))
	{
		if (ColliderShape* rect2 = dynamic_cast<ColliderShape*>(second))
		{
			return collisionTest(rect, rect2); // rectangle rectangle 
		}
		else if (CircleCollider* circ = dynamic_cast<CircleCollider*>(second))
		{
			return collisionTest(rect, circ); // rectangle circle
		}
	}

	if(CircleCollider* circ = dynamic_cast<CircleCollider*>(first))
	{
		if (ColliderShape* rect = dynamic_cast<ColliderShape*>(second))
		{
			return collisionTest(rect, circ); //rectangle circle
		}
		else if (CircleCollider* circ2 = dynamic_cast<CircleCollider*>(second))
		{
			return collisionTest(circ, circ2);  // circle circle
		}
	}


	return false;
}

bool Collision::checkPointEntityCollision(sf::Vector2f point, Point* entity)
{
	if (CircleCollider* circ = dynamic_cast<CircleCollider*>(entity))
	{
		return collisionTest(point, circ);
	}
	else if (ColliderShape* rect = dynamic_cast<ColliderShape*>(entity))
	{
		return collisionTest(point, rect);
	}

	return false;
}

bool Collision::checkAABBCollision(sf::RectangleShape & first_bounding_box, sf::RectangleShape &second_bounding_box) //ccurrently unused :(
{
	bool is_colliding = first_bounding_box.getGlobalBounds().findIntersection(second_bounding_box.getGlobalBounds()) ? true : false;

	return is_colliding;
}

bool Collision::checkAABBCollision(sf::FloatRect first_bounding_box, sf::FloatRect second_bounding_box)
{
	bool is_colliding = first_bounding_box.findIntersection(second_bounding_box) ? true : false;
	return is_colliding;
}

bool Collision::collisionTest(ColliderShape * first_collider, ColliderShape * second_collider)
{
	std::array<sf::Vector2f, 4> first_corners = first_collider->getTransformedCorners();
	std::array<sf::Vector2f, 4> second_corners = second_collider->getTransformedCorners();

	//axes of first shape

	for (sf::Vector2f edge : first_collider->getEdges())
	{
		sf::Vector2f normal = edge.perpendicular().normalized();

		sf::Vector2f p1 = project(normal, first_corners);
		sf::Vector2f p2 = project(normal, second_corners);

		if (!overlap(p1, p2))
		{
			return false;
		}

	}

	//axes of second shape

	for (sf::Vector2f edge : second_collider->getEdges())
	{
		sf::Vector2f normal = edge.perpendicular().normalized();

		sf::Vector2f p1 = project(normal, first_corners);
		sf::Vector2f p2 = project(normal, second_corners);

		if (!overlap(p1, p2))
		{
			return false;
		}

	}

	return true;
}

bool Collision::collisionTest(CircleCollider * first_collider, CircleCollider * second_collider)
{
	sf::Vector2f position1 = first_collider->getPosition();
	sf::Vector2f position2 = second_collider->getPosition();

	float r1 = first_collider->getRadius();
	float r2 = second_collider->getRadius();

	sf::Vector2f distance = position2 - position1;

	return  distance.length() <= (r1 + r2);
}

bool Collision::collisionTest(ColliderShape * first_collider, CircleCollider * second_collider)
{
	std::array<sf::Vector2f, 4> first_corners = first_collider->getTransformedCorners();

	for (sf::Vector2f edge : first_collider->getEdges())
	{
		sf::Vector2f normal = edge.perpendicular().normalized();

		sf::Vector2f p1 = project(normal, first_corners);

		sf::Vector2f p2 = project(normal, second_collider->getPosition(), second_collider->getRadius());


		if (!overlap(p1, p2))
		{
			return false;
		}

	}

	return true;
}

bool Collision::collisionTest(sf::Vector2f point, CircleCollider* collider)
{
	sf::Vector2f connecting_vector = point - collider->getPosition();

	float dist = connecting_vector.length();

	return dist <= collider->getRadius();
}

// Polyogn point collision, from https://www.jeffreythompson.org/collision-detection/poly-point.php accessed: 05/10/2025


bool Collision::collisionTest(sf::Vector2f point, ColliderShape* collider)
{
	bool collision = false;

	std::array<sf::Vector2f, 4> corners = collider->getTransformedCorners();
	
	sf::Vector2f tc, nc;

	int next = 0;

	for (int i = 0; i < corners.size(); i++)
	{
		next = i + 1;

		if (next == corners.size()) { next = 0; }

		tc = corners[i];
		nc = corners[next];

		if (((tc.y >= point.y && nc.y < point.y) || (tc.y < point.y && nc.y >= point.y)) && (point.x < (nc.x - tc.x) * (point.y - tc.y) / (nc.y - tc.y) + tc.x)) 
		{
			collision = !collision;
		}
	}


	return collision;
}

sf::Vector2f Collision::project(sf::Vector2f axis, std::array<sf::Vector2f, 4> vertices)
{
	float min = axis.dot(vertices[0]);
	float max = min;

	for (int i = 1; i < 4; i++)
	{
		float p = axis.dot(vertices[i]);
		if (p < min)
		{
			min = p;
		}
		else if (p > max)
		{
			max = p;
		}
	}

	return sf::Vector2f(min,max);
}

sf::Vector2f Collision::project(sf::Vector2f axis, sf::Vector2f point, float radius)
{
	float min = axis.dot(point) - radius;
	float max = axis.dot(point) + radius;

	return sf::Vector2f(min, max);
}

bool Collision::overlap(sf::Vector2f proj_one, sf::Vector2f proj_two)
{

	return !(proj_one.y < proj_two.x || proj_two.y < proj_one.x);
}

