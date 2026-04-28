#pragma once
#include <queue>
#include "SFML Framework/BaseGame.h"
#include "Player.h"
#include "Level.h"
#include "EnemyManager.h"
#include "Coin.h"
#include "NetworkManager.h"
#include "UI.h"

enum class RoundState {
	PreRound = 0,
	MidRound = 1,
	PostRound = 2
};

class Game : public BaseGame
{
public:
	Game(sf::RenderWindow* window);
	~Game();

	void handleInput(float fixed_timestep, Input* in);

	void fixedUpdate(float fixed_timestep) override;
	void networkUpdate(float fixed_timestep, NetworkManager* networkMgr); // input pass is temporary
	void handleCollisions(float fixed_timestep) override;
	void renderGame() override;

	void addPlayer(bool remote, Connection* connection);
	void removePlayer(uint8_t id);

	void nextRound();
	void beginRound(MatchUpdateMessage msg);
	void endRound();
	void endMatch();

	void resetGame(bool clearPoints = false);

	RoundState getRoundState() const;
	Player* getMostWealthyPlayer();

	static float getGameTime() { return gameTime; }
	static void setGameTime(float t) { gameTime = t; }

private:
	sf::Texture playerTextures[4];
	Player localPlayer;
	std::vector<Player> remotePlayers;
	std::vector<Player*> allPlayers;
	Player* getPlayerWithId(uint8_t id) const;
	uint8_t playerCount;

	Level level;
	BulletManager bulletManager;
	EnemyManager enemyManager;
	CoinManager coinManager;

	// AudioManager* audioManager;

	UI uiManager;

	sf::Clock gameTimeClock;
	static inline float gameTime = 0.0f;
	float roundTimer = 0.0f;

	static inline float playerDiedTime = -1.0f;

	unsigned int roundNumber = 0; // 0 = lobby
	int roundEnemyCount = 5; // changes at round start
	int enemiesToSpawn = roundEnemyCount; // decreases when enemy spawns
	int enemiesSlain = 0; // increases when enemy dies

	float difficultyModifier = 1;

	int8_t lastRoundWinner = -1;

	// events we will send over the network
	std::vector<GameEvent> eventsToSend;
	// events received from the network we need to process locally
	std::vector<GameEvent> eventsRecieved;
	// purely local events
	std::vector<GameEvent> localEvents;
};

