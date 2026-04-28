#include "Enemy.h"

void Enemy::reset()
{
	health = 6.0f;
	timeSinceHit = 1.0f;

	std::uniform_int_distribution<int> coinRange(1, 5);
	numCoins = coinRange(Random::engine);

	sf::Vector2f unit = { 1,0 };
	for (int i = 0; i < numCoins; i++) {
		coinDirs[i] = unit.rotatedBy(sf::degrees(rand() % 360));
	}
	worth = static_cast<float>((rand() % 10)+5);

	for (int i = 0; i < MAX_PLAYERS; i++) {
		aggro[i] = 0;
	}

	interpDestination.reset();
	timeRequired = 0.0f; interpCompletion = 0.0f;

	isActive = true;
}

void Enemy::fixedUpdate(float dt, std::vector<Player*>& players)
{
	// update immune attacks deque
	if (immuneBullets.size() == 0) return;

	// add dt to the time since attack, and see if it's greater than the immune time
	int i = 0;
	for (i = 0; i < immuneBullets.size(); i++) {
		immuneBullets[i].timeSinceAttack += dt;
		if (immuneBullets[i].timeSinceAttack > ENEMY_IMMUNE_TIME) {
			break;
		}
	}

	// every bullet from here on must have expired immunity once dt is added, so pop back
	for (i = i; i < immuneBullets.size(); i++) {
		immuneBullets.pop_back();
	}
}

void Enemy::takeDamage(Bullet* bullet)
{
	timeSinceHit = 0.0f;

	// only local bullets can deal damage
	if (bullet->local) {
		health -= bullet->damage;
		aggro[bullet->firedPlayerId] += (BASE_ATTACK_AGGRO + bullet->damage);
	}

	if (bullet->pierce == 0) {
		bullet->isActive = false;
	}
	// we only need to be immune to bullets that continue to exist after hitting, if we're alive
	else if (isActive) {
		bullet->pierce--;
		immuneBullets.push_front({bullet});
	}
}

void Enemy::takeDamage(float damage, uint8_t attackerId)
{
	timeSinceHit = 0.0f;
	health -= damage;
	aggro[attackerId] += (BASE_ATTACK_AGGRO + damage);

	// don't die if health < 0, just despawn, to prevent coin drops
	if (health <= 0) {
		isActive = false;
	}
}

void Enemy::die(std::vector<GameEvent>* events)
{
	isActive = false;
	sf::Vector2f pos = getWorldCenter();
	int val = 1;
	
	if (events == nullptr) return;
	CoinsSpawnRequestMessage csrm;
	csrm.num_coins = numCoins;
	csrm.position = getPosition();
	csrm.total_worth = worth;
	csrm.enemy = true;

	events->push_back({ CoinsSpawnRequest, csrm });
	Utils::printMsg("requested coin spawn", debug);

	// todo: total coin value can still be over enemy worth (not much of an issue)
	//for (int i = 0; i < numCoins; i++) {
	//	val = static_cast<int>(round(worth / numCoins));
	//	if (val < 1) val = 1;

	//	// if this is the last coin, add value to match enemy worth
	//	coinsValue -= val;
	//	if (i == numCoins - 1 && coinsValue > 0) val += coinsValue;
	//	coinMgr->spawnCoin(pos, coinDirs[i], val);
	//}
}

void Enemy::applyNetworkUpdate(sf::Vector2f destination, float time)
{
	interpDestination = std::make_optional<sf::Vector2f>(destination);
	interpOrigin = getPosition();
	timeRequired = time;
	interpCompletion = 0.0f;
}

Player& Enemy::decideTarget(std::vector<Player*>& players)
{
	// player chosen is the closest for (distance-aggro)
	auto it = std::min_element(players.begin(), players.end(), [this](Player* plr0, Player* plr1) {
		float distSqA = (plr0->getPosition() - getPosition()).lengthSquared();
		distSqA -= pow(aggro[plr0->getId()], 2);
		float distSqB = (plr1->getPosition() - getPosition()).lengthSquared();
		distSqB -= pow(aggro[plr1->getId()], 2);

		return distSqA < distSqB;
		});
	return **it;
}

///////////////////////// ^ Enemy ^ ////////////////////////////
////////////////////////////////////////////////////////////////
/////////////////////// v GhostEnemy v /////////////////////////

GhostEnemy::GhostEnemy(sf::Texture* tex)
{
	setTexture(tex);
	setSize({ 36, 52 });
	setCollisionRect({ { 0,0 }, { 36, 52 } });

	moveAnim.addFramesRow({{ 0, 0 }, { 36, 52 }}, 4);
	moveAnim.setAnimationFps(4.0f);
}

void GhostEnemy::fixedUpdate(float dt, std::vector<Player*>& players, float diffMod)
{
	Enemy::fixedUpdate(dt, players);

	sf::Vector2f dir;

	// diffmod needs to be different for movement or it ramps WAY too fast
	// this formula ramps it half as fast
	float movementMod = 1.0f + ((diffMod - 1) / 2.0f);
	Utils::printMsg(std::to_string(movementMod));
	
	// interpolate if we should
	// (this is a network function, so doesn't happen in audio project)
	if (interpDestination.has_value() && interpCompletion < 1.0f) {
		sf::Vector2f diff = (interpDestination.value() - getPosition());
		if (diff.lengthSquared() != 0) {
			dir = diff.normalized();
		}
		else {
			dir = sf::Vector2f(1.0f, 0.0f);
		}

		float step = 1.0f / (timeRequired / dt);
		interpCompletion += step;

		setPosition(interpCompletion * interpDestination.value() + (1 - interpCompletion) * interpOrigin);

		if ((getPosition() - interpDestination.value()).lengthSquared() <= ENEMY_DESYNC_DISTANCE_SQUARED) {
			interpDestination.reset();
		}
	}

	// predict otherwise (or just simulate if host)
	else {

		sf::Vector2f target = Enemy::decideTarget(players).getWorldCenter();
		sf::Vector2f diff = (target - getWorldCenter());
		if (diff.lengthSquared() > 0.0f) {
			dir = diff.normalized();
		}
		else {
			dir = sf::Vector2f(1.0f, 0.0f);
		}
		velocity += (dir * GHOST_ACCEL * dt * movementMod);

		// cap velocity
		if (velocity.lengthSquared() > GHOST_MAX_SPEED_SQUARED * (movementMod * movementMod)) {
			velocity = (velocity.normalized() * GHOST_MAX_SPEED * movementMod);
		}

		move(velocity * dt);
	}

	moveAnim.animate(dt);
	moveAnim.setHorizontalFlip(dir.x > 0.0f);
	setTextureRect(moveAnim.getCurrentFrame());

	timeSinceHit += dt;
	if (isRecentlyHit()) { setFillColor({ 255,0,0 }); }
	else { setFillColor({ 255,255,255 }); }
}