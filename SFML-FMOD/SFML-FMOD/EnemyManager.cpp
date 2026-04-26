#include "EnemyManager.h"

EnemyManager::EnemyManager()
{
	if (!ghostTexture.loadFromFile("gfx/ghosts.png")) {
		throw std::invalid_argument("ghost texture was not found!");
	}

	ghosts = std::vector<GhostEnemy>(MAX_NUM_ENEMY, GhostEnemy(&ghostTexture));
	for (int i = 0; i < MAX_NUM_ENEMY; i++) { ghosts[i].id = i; }
}

void EnemyManager::spawnEnemy(std::vector<GameEvent>* eventsToSend)
{
	GhostEnemy* g = getFirstEnemy();
	if (g == nullptr) return;

	timeSinceSpawn = 0.0f;

	Random::newRandomSeed();
	sf::Vector2f pos = enemySpawnPoints[Random::engine() % enemySpawnPoints.size()];
	g->setPosition(pos);
	g->reset();

	if (eventsToSend == nullptr) return;
	EnemySpawnMessage esmsg;
	esmsg.enemy_id = g->id;
	esmsg.seed = Random::randSeed;
	eventsToSend->push_back({ EnemySpawn, esmsg });

	Utils::printMsg("spawning ghost w params: coin num " + std::to_string(g->numCoins) + ", health " + std::to_string(g->health) + ", pos " 
		+ std::to_string(g->getPosition().x) + "," + std::to_string(g->getPosition().y), debug);
}

void EnemyManager::spawnEnemy(EnemySpawnMessage esmsg)
{
	GhostEnemy* g = &ghosts.at(esmsg.enemy_id);

	Random::engine.seed(esmsg.seed);
	sf::Vector2f pos = enemySpawnPoints[Random::engine() % enemySpawnPoints.size()];
	g->setPosition(pos);
	g->reset();

	Utils::printMsg("spawning ghost w params: coin num " + std::to_string(g->numCoins) + ", health " + std::to_string(g->health) + ", pos " 
		+ std::to_string(g->getPosition().x) + "," + std::to_string(g->getPosition().y), debug);
}

void EnemyManager::update(float dt, bool host, std::vector<Player*>& players, std::vector<GameEvent>* localEvents, std::vector<GameEvent>* eventsToSend)
{
	timeSinceSpawn += dt;
	if (canSpawnEnemy() && host) {
		spawnEnemy(eventsToSend);
	}

	for (auto& g : ghosts) {
		if (!g.isActive) continue;
		if (g.health <= 0.0f) {
			if (host) g.die(localEvents);
			else g.die(eventsToSend);
		}
		else { g.fixedUpdate(dt, players); }
	}
}

void EnemyManager::networkUpdate(GhostsUpdateMessage gumsg)
{
	// todo: interp
	for (int i = 0; i < gumsg.num_ghosts; i++) {
		GhostUpdate currentUpdate = gumsg.updates[i];

		// teleport if outside of the tolerance range
		//if ((ghosts[currentUpdate.ghost_id].getPosition() - currentUpdate.position).lengthSquared() > ENEMY_DESYNC_DISTANCE_SQUARED) {
		//	ghosts[currentUpdate.ghost_id].setPosition(currentUpdate.position);
		//}
		
		//ghosts[currentUpdate.ghost_id].interpDestination = std::make_optional(currentUpdate.position);

		ghosts[currentUpdate.ghost_id].applyNetworkUpdate(currentUpdate.position, gumsg.time_required);
	}
}

void EnemyManager::handleLevelCollision(std::vector<sf::FloatRect> platforms)
{
	// ghosts don't have collision!
}

void EnemyManager::handleBulletCollision(std::vector<Bullet*> bullets, std::vector<GameEvent>* eventsToSend)
{
	for (auto& g : ghosts) {
		if (!g.isActive) continue;
		for (auto b : bullets) {
			if (g.getCollisionRect().findIntersection(b->getCollisionRect()).has_value()) {
				g.takeDamage(b);
				
				// only local bullets should spawn messages
				if (b->local) {
					EnemyHurtMessage ehm(g.id, b->damage, b->firedPlayerId);
					eventsToSend->push_back({ EnemyHurt, ehm });
				}
			}
		}
	}
}

void EnemyManager::handlePlayerCollision(Player& player, std::vector<GameEvent>* coinSpawnRequestList)
{
	for (auto& g : ghosts) {
		if (!g.isActive) continue;
		if (g.getCollisionRect().findIntersection(player.getCollisionRect()).has_value()) {
			player.takeDamage(g.getWorldCenter().x, coinSpawnRequestList, true);
		}
	}
}

void EnemyManager::hurtEnemy(EnemyHurtMessage ehmsg)
{
	ghosts.at(ehmsg.enemy_id).takeDamage(ehmsg.damage_taken, ehmsg.attacking_player_id);
}

void EnemyManager::render(sf::RenderWindow* wn)
{
	for (auto& g : ghosts) {
		if (!g.isActive) continue;
		wn->draw(g);
	}
}

void EnemyManager::clearEnemies()
{
	for (auto& g : ghosts) {
		g.isActive = false;
	}
}

std::vector<GhostEnemy*> EnemyManager::getAliveEnemies()
{
	std::vector<GhostEnemy*> alive;
	for (auto& g : ghosts) {
		if (g.isActive) alive.push_back(&g);
	}
	return alive;
}

GhostsUpdateMessage EnemyManager::createUpdateMessage()
{
	GhostsUpdateMessage msg;
	for (auto& g : ghosts) {
		if (g.isActive) {
			msg.updates[msg.num_ghosts] = { g.id, g.getPosition() };
			msg.num_ghosts++;
		}
	}
	return msg;
}

GhostEnemy* EnemyManager::getFirstEnemy(bool active)
{
	for (auto& g : ghosts) {
		if (g.isActive == active) return &g;
	}

	return nullptr;
}
