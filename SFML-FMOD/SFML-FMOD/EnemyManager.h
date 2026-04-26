#pragma once
#include "Enemy.h"
#include "Player.h"
#include "Coin.h"
#include <memory>

class EnemyManager
{
public:
	EnemyManager();
	~EnemyManager() {};

	void spawnEnemy(std::vector<GameEvent>* eventsToSend = nullptr);	// spawn enemy locally and make an event from it
	void spawnEnemy(EnemySpawnMessage esmsg);							// spawn enemy from received event

	// update all active enemies
	void update(float dt, bool host, std::vector<Player*>& players, std::vector<GameEvent>* localEvents, std::vector<GameEvent>* eventsToSend = nullptr);

	void networkUpdate(GhostsUpdateMessage gumsg);						// interpolate ghost positions
	void handleLevelCollision(std::vector<sf::FloatRect> platforms);	// collision between enemies and platforms (UNUSED)
	void handleBulletCollision(std::vector<Bullet*> bullets, std::vector<GameEvent>* eventsToSend); // collision between enemies and bullets
	void handlePlayerCollision(Player& player, std::vector<GameEvent>* coinSpawnRequestList); // collision between enemies and players
	void hurtEnemy(EnemyHurtMessage ehmsg);								// deal damage to an enemy from an event
	void render(sf::RenderWindow* wn);									// draw all active enemies

	void toggleSpawn(bool spawn) { spawnsEnabled = spawn; }
	void clearEnemies();
	std::vector<GhostEnemy*> getAliveEnemies();							// vector of all active enemies
	GhostsUpdateMessage createUpdateMessage();							// create an update message of ghost positions

private:
	inline static constexpr float spawnRate = 2.5f;
	float timeSinceSpawn = 0.0f;
	bool spawnsEnabled = false;
	bool canSpawnEnemy() const { return timeSinceSpawn > spawnRate && spawnsEnabled; }

	// todo: may need to make not const and read from level data
	const std::vector<sf::Vector2f> enemySpawnPoints { 
		{42.0f, 42.0f}, 
		{840.0f, 42.0f},
		{483.0f, 42.0f},
		{42.0f, 840.0f},
		{840.0f, 840.0f}
	};

	// if more enemy types, make this unique ptr to base class. fine for now
	std::vector<GhostEnemy> ghosts;
	sf::Texture ghostTexture;

	// get the earliest enemy in the vector matching specified active state, or nullptr if none
	GhostEnemy* getFirstEnemy(bool active = false);
};

