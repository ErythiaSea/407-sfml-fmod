#pragma once
#include <SFML/Graphics.hpp>
#include "ColliderShape.h"
#include "CollisionEntity.h"
#include "CircleCollider.h"

class Collision
{
public:
	static bool checkCollision(Point* first, Point* second);
	static bool checkPointEntityCollision(sf::Vector2f point, Point* entity);
	static bool checkAABBCollision(sf::RectangleShape &first_bounding_box, sf::RectangleShape &second_bounding_box); // currently unused
	static bool checkAABBCollision(sf::FloatRect first_bounding_box, sf::FloatRect second_bounding_box);
private:

	static bool collisionTest(ColliderShape * first_collider, ColliderShape * second_collider); //rectangle/rectangle collision
	static bool collisionTest(CircleCollider * first_collider, CircleCollider * second_collider); //circle/circle collision
	static bool collisionTest(ColliderShape * first_collider, CircleCollider * second_collider); //rectangle/circle collision

	static bool collisionTest(sf::Vector2f point, CircleCollider* collider); //point/circle collision
	static bool collisionTest(sf::Vector2f point, ColliderShape* collider); //point/rectangle collision

	//functions for SAT collision
	static sf::Vector2f project(sf::Vector2f axis, std::array<sf::Vector2f, 4> vertices);
	static sf::Vector2f project(sf::Vector2f axis, sf::Vector2f point, float radius);

	static bool overlap(sf::Vector2f proj_one, sf::Vector2f proj_two);
};

