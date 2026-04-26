#pragma once
#include "SFML Framework/Entity.h"
#include "SFML Framework/Input.h"
#include <stdexcept>

class Bullet : public Entity
{
public:
	Bullet(sf::Texture* tex);
	~Bullet() {};
	void fixedUpdate(float dt) override;

	// velocity of the bullet
	sf::Vector2f velocity;

	// damage of the bullet (modified when fired)
	float damage = 1.0f;

	// how many enemies the bullet can travel through (currently unused)
	int pierce = 0;
	
	// the id of the player that fired the bullet
	uint8_t firedPlayerId;

	// did the bullet come from a local player?
	bool local = false;

	// is the bullet currently being processed?
	bool isActive = false;

	void resetLifeTime() { lifeTime = 1.5f; }
private:
	// how long the bullet has left until being destroyed
	float lifeTime = 1.5f;
};

class BulletManager {
public:
	BulletManager();
	~BulletManager() {};

	void spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float vel, uint8_t playerId, bool isLocal);

	void clearBullets();

	// update all active bullets
	void update(float dt);

	// handle collision between bullets and platforms
	void handleLevelCollision(std::vector<sf::FloatRect> platforms);

	// draw all active bullets
	void render(sf::RenderWindow* wn);

	// get a pointer to every bullet that's active
	std::vector<Bullet*> getAliveBullets();

private:
	// every bullet
	std::vector<Bullet> bullets;

	// the texture for bullets
	sf::Texture bulletTexture;

	// get the earliest bullet in the vector matching specified active state
	Bullet* getFirstBullet(bool active = false);
};
