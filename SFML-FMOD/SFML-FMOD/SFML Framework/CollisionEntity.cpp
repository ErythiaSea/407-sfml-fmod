#include "CollisionEntity.h"

#include <iostream>

CollisionEntity::CollisionEntity()
{
	collider.setSize(sf::Vector2f(100,100));
	collider.setCentre(sf::Vector2f(50, 50));
}

CollisionEntity::~CollisionEntity()
{
}

void CollisionEntity::fixedUpdate(float dt)
{

}

void CollisionEntity::variableUpdate(float dt)
{
}

ColliderShape* CollisionEntity::getCollider()
{
	collider.update(getTransform());
	return &collider;
}
