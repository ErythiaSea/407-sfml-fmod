#pragma once
#include "SFML Framework/CollisionEntity.h"
#include "SFML Framework/Animation.h"
#include "SFML Framework/Input.h"
#include "FMODManager.h"
#include "Bullet.h"
#include "utils.h"
#include "Messages.h"
#include "Connection.h"
#include "Constants.h"

class Player : public Entity
{
public:
	Player(BulletManager* bm, sf::Texture& tex, std::vector<GameEvent>& events, Connection* con = nullptr);
	~Player(){};

	MovementInputsMessage handleInput(float dt, Input* in);
	void simulateInput(float dt);

	void fixedUpdate(float dt) override;
	void remoteUpdate(float dt);
	void handleLevelCollision(std::vector<sf::FloatRect> platforms);
	void postCollisionUpdate(float dt);

	void takeDamage(float attackX, std::vector<GameEvent>* coinSpawnRequestList, bool addFlag = false, float knockback = 3.0f, bool ignoreStun = false);

	bool isFacingRight() const { return facingRight; }
	bool isStunned() const { return timeSinceHit < STUN_TIME; }
	bool isImmune() const { return timeSinceHit < IFRAME_TIME; }
	bool canCollectCoins() const { return timeSinceHit > HURT_COIN_PICKUP_DISABLE_TIME; }

	void addMoney(const int sum) { money += sum; }
	int getMoney() const { return money; }
	void setMoney(int val = 0) { money = val; }

	void addPoint() { points++; }
	void clearPoints() { points = 0; }
	uint8_t getPoints() const { return points; }

	void setHealth(const int h) { health = h; }
	int getHealth() const { return health; }

	void setId(const uint8_t new_id) { id = new_id; }
	uint8_t getId() const { return id; }

	Connection* getConnection() const { return connection; }
	void setConnection(Connection* con) { connection = con; }

	PlayerUpdateMessage createUpdateMsg() const;
	void addUpdateMessage(PlayerUpdateMessage msg);
	void addInputsMessage(MovementInputsMessage msg);
	void clearInputsHistory() { movementInputsHistory.clear(); }

	void setRateOfFire(float rate) { rateOfFire = rate; }

protected:
	inline static constexpr float ACCEL = 5.0f;
	inline static constexpr float MAX_HORIZONTAL_SPEED = SCALE * 1.5f;
	inline static constexpr float STUN_TIME = 0.33f;
	inline static constexpr float IFRAME_TIME = 1.33f;
	inline static constexpr float HURT_COIN_PICKUP_DISABLE_TIME = 1.0f;
	inline static constexpr int MAX_AIR_JUMPS = 1;
	inline static constexpr float COYOTE_TIME = 0.12f;
	inline static constexpr int MAX_LIFE = 3;
	inline static constexpr sf::Vector2f LEFT_BULLET_OFFSET = sf::Vector2f(10, 26);
	inline static constexpr sf::Vector2f RIGHT_BULLET_OFFSET = sf::Vector2f(20, 26);

	Animation walk;
	Animation fall;
	Animation jump;
	Animation hurt;
	Animation* currentAnim;
	void decideAnimationState();
	bool facingRight = true;

	float throttle = 0.0f;

	bool isGrounded = false;
	float coyoteTimer = COYOTE_TIME;
	void setGrounded(bool grounded);
	int airJumpsRemaining = 0;

	int money = 0;
	int health = MAX_LIFE;
	float timeSinceHit = 1.0f;

	float rateOfFire = BASE_FIRE_RATE;
	float timeSinceFired = 1.0f;

	// platforms colliding with the player
	int numColliders = 0;
	std::vector<sf::FloatRect> collidingRects;
	void addCollider(sf::FloatRect rect);

	BulletManager* bulletManager;

	uint8_t points = 0;

	uint8_t id = 0;
	std::vector<GameEvent>* eventsToSendList;

	// this will be nullptr if it's a local player
	Connection* connection = nullptr;
	std::deque<PlayerUpdateMessage> messageHistory;
	std::deque<MovementInputsMessage> movementInputsHistory;
};