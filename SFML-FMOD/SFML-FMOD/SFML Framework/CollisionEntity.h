#pragma once
#include "Entity.h"
#include "ColliderShape.h"

class CollisionEntity :public Entity
{

public:
	CollisionEntity();
	~CollisionEntity();


	void fixedUpdate(float dt) override;
	void variableUpdate(float dt) override;

	ColliderShape* getCollider();

private:
	ColliderShape collider;
};

