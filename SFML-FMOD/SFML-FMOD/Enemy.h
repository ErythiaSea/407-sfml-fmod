#pragma once
#include "SFML Framework/Entity.h"
#include "SFML Framework/Animation.h"
#include "Bullet.h"
#include "Coin.h"
#include "FMODManager.h"
#include <random>
#include <deque>
#include <cmath>

// a struct that contains a bullet and the time since it hit the enemy
// used to prevent multiple hits from piercing bullets
struct Attack {
	Bullet* bullet;
	float timeSinceAttack;

	Attack(Bullet* b) { 
		bullet = b;
		timeSinceAttack = 0.0f;
	}
};

class Enemy : public Entity
{
public:
	float health = 10;						// the enemy's health
	float timeSinceHit = 1.0f;				// time since the enemy was last hit, for visual effects
	bool isActive = false;					// if the enemy is alive and being processed
	float worth = 1.0f;						// total money the enemy drops before rounding
	int numCoins = 1;						// how many coins drop when it dies
	std::array<sf::Vector2f, 5> coinDirs;	// the initial directions of dropped coins

	// reset enemy values to their defaults to be spawned anew
	void reset();

	// update position (chase after a player)
	void fixedUpdate(float dt, std::vector<Player*>& players);

	// take damage from a bullet (and become temporarily immune to it, if it pierces)
	void takeDamage(Bullet* bullet);

	// take an arbitrary amount of damage
	void takeDamage(float damage, uint8_t attackerId);

	// despawn and drop coins
	void die(std::vector<GameEvent>* events);

	// determine if the enemy was hit recently
	bool isRecentlyHit() const { return timeSinceHit < ENEMY_HURT_TIME; }

	// set the values to start interpolation
	void applyNetworkUpdate(sf::Vector2f destination, float time);

protected:
	std::deque<Attack> immuneBullets;								// bullets that pierced the enemy recently
	static inline constexpr float ENEMY_HURT_TIME = 0.1f;			// for visual effects
	static inline constexpr float ENEMY_IMMUNE_TIME = 1.0f / 6.0f;	// how long an enemy will be immune to an attack for

	std::optional<sf::Vector2f> interpDestination;				// the destination for interpolation
	sf::Vector2f interpOrigin;									// where to interpolate from
	float timeRequired, interpCompletion = -1.0f;				// the time to interpolate for, and current interp completion

	std::unordered_map<uint8_t, int32_t> aggro;					// aggro values, key is the player id
	Player& decideTarget(std::vector<Player*>& players);		// deciding which player to target based on distance and aggro
};

class GhostEnemy : public Enemy
{
public:
	GhostEnemy(sf::Texture* tex);
	
	// update the ghost enemy (move towards a player)
	void fixedUpdate(float dt, std::vector<Player*>& players, float diffMod);

	sf::Vector2f velocity;		// ghost's velocity
	Animation moveAnim;			// ghost default animation
	uint8_t id = 0;				// ghost's id (position in vector)

private:
	static inline constexpr float GHOST_ACCEL = 480.0f;			// rate at which a ghost accels
	static inline constexpr float GHOST_MAX_SPEED = 120.0f;		// speed at which a ghost can move
	static inline constexpr float GHOST_MAX_SPEED_SQUARED = GHOST_MAX_SPEED * GHOST_MAX_SPEED;	// for length comparison
};
