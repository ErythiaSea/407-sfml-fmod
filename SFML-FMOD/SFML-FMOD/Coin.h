#pragma once
#include "SFML Framework/Entity.h"
#include "SFML Framework/Animation.h"
#include "Player.h"
#include <algorithm>

class Coin : public Entity
{
public:
	Coin(sf::Texture* tex);
	~Coin(){};

	// update the coin's position
	void fixedUpdate(float dt);
	
	// handle collisions with platforms
	void collisionResponse(std::vector<sf::FloatRect> platforms, int count);

	// reset values of the coin to be spawned again
	void reset();

	uint8_t id = 0;

	sf::Vector2f velocity;			// coin's velocity
	int value = 1;					// how much the coin is worth
	bool isGrounded = false;		// if the coin is touching the ground
	bool fromPlayer = false;		// if the coin was dropped by a hurt player
	bool followingPlayer = false;	// if the coin is under the effects of magnesis (UNUSED)

	// animations for different coin types
	Animation copper;
	Animation silver;				// UNUSED
	Animation gold;					// UNUSED

	// pointer to the current animation
	Animation* currentAnim;

	bool isActive = false;			// if the coin is being processed
};

class CoinManager
{
public:
	CoinManager();
	~CoinManager() {};

	// spawn a new coin 
	//void spawnCoin(uint8_t id);

	// spawn several coins from a CoinsSpawnMessage
	void spawnCoins(CoinsSpawnMessage csm);

	void despawnCoin(uint8_t id) { coins.at(id).isActive = false; }

	// clear all active coins
	void clearCoins();

	// update all active coins
	void update(float dt);

	// process pending coin spawn requests, either from network or local
	void processSpawnRequest(CoinsSpawnRequestMessage csrm, std::vector<GameEvent>& eventsToSend);

	// handle collision between coins and platforms
	void handleLevelCollision(std::vector<sf::FloatRect> platforms);

	// handle collision between coins and players
	void handlePlayerCollision(Player& player, std::vector<GameEvent>& eventsToSend);

	// draw all active coins
	void render(sf::RenderWindow* wn);

private:
	std::vector<Coin> coins;		// all coins
	sf::Texture coinTexture;		// texture for coins

	// get the earliest coin in the vector matching specified active state,
	// or nullptr if none exist
	Coin* getFirstCoin(bool active = false);

	// get the vector index where n coins match the active state,
	// or -1 if there's none
	int8_t getBlockOfNCoins(size_t n, bool active = false);

	// stores platforms that collide with each coin
	std::vector<sf::FloatRect> collidingPlatforms;
	void addCollider(sf::FloatRect rect, int num);
};