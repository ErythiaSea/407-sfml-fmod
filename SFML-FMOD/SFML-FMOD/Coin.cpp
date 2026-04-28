#include "Coin.h"

Coin::Coin(sf::Texture* tex)
{
	setTexture(tex);
	setSize({ 12,16 });
	setCollisionRect({ {0, 4}, {12, 12} });

	copper.addFrame({ {4,0},{12,16} });
	silver.addFrame({ {21, 0},{12,16} });
	gold.addFrame({ {39, 0},{12,16} });

	currentAnim = &copper;
	setTextureRect(currentAnim->getCurrentFrame());
}

void Coin::fixedUpdate(float dt)
{
	// skip updating a coin if its velocity is effectively zero
	if (velocity.lengthSquared() < 0.0001f && isGrounded) return;

	// apply gravity and friction
	velocity.y += GRAVITY * SCALE * dt;
	if (isGrounded) {
		velocity.x = velocity.x / 1.25f;
	}

	if (velocity.y > MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;
	move(velocity * dt);

	// out of bounds
	if (getPosition().y > 1000.0f) {
		isActive = false;
	}
}

void Coin::collisionResponse(std::vector<sf::FloatRect> platforms, int count)
{
	if (count == 0) return;

	// run twice to account for corners, unless only one collider
	int iter = (count == 1 ? 1 : 2);
	for (int i = 0; i < iter; i++) {
		auto end = platforms.begin() + (count);
		sf::FloatRect thisColRect = getCollisionRect();

		// get the largest collision rectangle by area
		// todo: probably optimisable by calcing intersect rects once and storing
		sf::FloatRect mostOverlap;
		if (count > 1) {
			auto result = std::max_element(platforms.begin(), end, [thisColRect](sf::FloatRect a, sf::FloatRect b) {
				sf::FloatRect ca = thisColRect.findIntersection(a).value();
				sf::FloatRect cb = thisColRect.findIntersection(b).value();
				return (ca.size.x * ca.size.y) < (cb.size.x * cb.size.y);
				});
			mostOverlap = thisColRect.findIntersection(*result).value();

			// need to erase this rect since we wont be colliding with it anymore
			platforms.erase(result);
			count--;
		}
		else {
			mostOverlap = thisColRect.findIntersection(platforms[0]).value();
		}

		isGrounded = false;
		// x collision
		if (mostOverlap.size.x < mostOverlap.size.y) {
			if (velocity.x > 0.0f) { // moving right
				move({ -mostOverlap.size.x, 0.0f });
			}
			else { // moving left
				move({ mostOverlap.size.x, 0.0f });
			}
			velocity.x = 0.0f;
		}
		else { // y collision
			if (velocity.y > 0.0f) { // moving down
				move({ 0.0f, -mostOverlap.size.y });
				isGrounded = true;
			}
			else { // moving up
				move({ 0.0f, mostOverlap.size.y });
			}
			velocity.y = 0.0f;
		}
	}
}

void Coin::reset()
{
	isGrounded = false;
	fromPlayer = false;
	followingPlayer = false;

	isActive = true;
}

////////////////////////// ^ Coin ^ /////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////// v CoinManager v /////////////////////////

CoinManager::CoinManager()
{
	if (!coinTexture.loadFromFile("gfx/coins.png")) {
		throw std::invalid_argument("coin texture was not found!");
	}

	coins = std::vector<Coin>(MAX_NUM_COINS, Coin(&coinTexture));
	for (int i = 0; i < MAX_NUM_COINS; i++) { coins[i].id = i; }

	coinDespawnClock = sf::Clock();
}

//void CoinManager::spawnCoin(uint8_t id)
//{
//	Coin* c = getFirstCoin();
//
//	c->setPosition(pos);
//	c->velocity = dir * 175.0f;
//	c->value = val;
//	c->reset();
//}

void CoinManager::spawnCoins(CoinsSpawnMessage csm)
{
	Random::engine.seed(csm.seed);
	int coinsValue = static_cast<int>(csm.total_worth);
	const sf::Vector2f unit = { 1,0 };

	for (uint8_t i = 0; i < csm.num_coins; i++) {
		Coin& c = coins.at(csm.coin_ids[i]);
		c.setPosition(csm.position);

		// calculate value
		int val = static_cast<int>(round(csm.total_worth / csm.num_coins));
		if (val < 1) val = 1;

		// if this is the last coin, add value to match total worth
		coinsValue -= val;
		if (i == csm.num_coins - 1 && coinsValue > 0) val += coinsValue;
		c.value = val;

		// apply velocity at random angle
		sf::Vector2f dir = unit.rotatedBy(sf::degrees(Random::engine() % 360));
		c.velocity = dir * COIN_SPAWN_VELOCITY;

		c.isActive = true;
	}
}

void CoinManager::clearCoins(float time)
{
	if (time != 0) {
		coinDespawnClock.restart();
		coinDespawnTime = time;
		return;
	}

	for (auto& c : coins) {
		c.isActive = false;
	}
}

void CoinManager::update(float dt)
{
	for (auto& c : coins) {
		if (!c.isActive) continue;
		c.fixedUpdate(dt);
	}
	if (coinDespawnClock.getElapsedTime().asSeconds() > coinDespawnTime) {
		clearCoins();
		coinDespawnClock.reset();
		coinDespawnTime = 1;
	}
}

void CoinManager::processSpawnRequest(CoinsSpawnRequestMessage csrm, std::vector<GameEvent>& eventsToSend)
{
	// no coins to spawn
	if (csrm.num_coins == 0 || csrm.total_worth == 0) return;

	CoinsSpawnMessage spawnMsg;

	spawnMsg.num_coins = csrm.num_coins;
	spawnMsg.position = csrm.position;
	spawnMsg.total_worth = csrm.total_worth;

	Random::newRandomSeed();
	spawnMsg.seed = Random::randSeed;

	for (uint8_t i = 0; i < csrm.num_coins; i++) {
		Coin* freeCoin = getFirstCoin();
		if (freeCoin == nullptr) {
			spawnMsg.num_coins = i;
			break;
		}

		spawnMsg.coin_ids[i] = freeCoin->id;
		freeCoin->isActive = true;
	}

	eventsToSend.push_back({ CoinsSpawn, spawnMsg });
	spawnCoins(spawnMsg);
}

void CoinManager::handleLevelCollision(std::vector<sf::FloatRect> platforms)
{
	int collisionCount = 0;
	for (auto& c : coins) {
		if (!c.isActive) continue;
		collisionCount = 0;
		for (auto& p : platforms) {
			if (p.findIntersection(c.getCollisionRect()).has_value()) {
				addCollider(p, collisionCount);
				collisionCount++;
			}
		}
		if (collisionCount != 0) {
			c.collisionResponse(collidingPlatforms, collisionCount);
		}
	}
}

void CoinManager::handlePlayerCollision(Player& player, std::vector<GameEvent>& eventsToSend)
{
	for (auto& c : coins) {
		if (!c.isActive) continue;
		if (player.getCollisionRect().findIntersection(c.getCollisionRect()).has_value()) {
			player.addMoney(c.value);
			std::cout << player.getMoney() << "\n";
			c.isActive = false;
			eventsToSend.push_back({ CoinPickup, CoinPickupMessage(c.id) });
		}
	}
}

void CoinManager::render(sf::RenderWindow* wn)
{
	for (auto& c : coins) {
		if (!c.isActive) continue;
		wn->draw(c);
	}
}

Coin* CoinManager::getFirstCoin(bool active)
{
	for (auto& c : coins) {
		if (c.isActive == active) return &c;
	}

	return nullptr;

	// resize vector if coin max hit (UNUSED)
	//if (coins.size() < 512) {
	//	int originalSize = coins.size();
	//	coins.resize(coins.size() * 2, Coin(&coinTexture));
	//	return &coins[originalSize];
	//}

	// if all coins are active, rotate vector and get the last element
	// (i can't do this in a networked game unless i signal to all players to rotate the vector. UNUSED!)
	//std::rotate(coins.begin(), coins.begin() + 1, coins.end());
	//return &coins.back();
}

int8_t CoinManager::getBlockOfNCoins(size_t n, bool active)
{
	int8_t count = 0;
	int8_t current_index = 0;
	int8_t potential_index = -1;

	for (auto& c : coins) {
		if (c.isActive == active) {
			count++;
			if (potential_index == -1) {
				potential_index = current_index;
			}
			if (count == n) {
				return potential_index;
			}
		}
		else {
			count = 0;
			potential_index = -1;
		}
		current_index++;
	}

	return potential_index;
}

void CoinManager::addCollider(sf::FloatRect rect, int num)
{
	// if we can't fit this in the vector, push it back to make space
	if (collidingPlatforms.size() < num + 1) {
		collidingPlatforms.push_back(rect);
	}
	// otherwise write this in place over what was there before
	else {
		collidingPlatforms[num] = rect;
	}
}
