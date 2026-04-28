#include "Bullet.h"

Bullet::Bullet(sf::Texture* tex)
{
	setTexture(tex);
	setSize(sf::Vector2f(4, 58));
	setCollisionRect({ { -0, -0 }, {4, 4} });

	// so that bullets rotate around the center of the bullet
	setOrigin({2,2});
}

void Bullet::fixedUpdate(float dt)
{
	lifeTime -= dt;
	if (lifeTime <= 0.0f) {
		isActive = false;
		return;
	}

	move(velocity * dt);
}

////////////////////////// ^ Bullet ^ /////////////////////////////
///////////////////////////////////////////////////////////////////
/////////////////////// v BulletManager v /////////////////////////

BulletManager::BulletManager()
{
	if (!bulletTexture.loadFromFile("gfx/bullet.png")){
		throw std::invalid_argument("bullet texture was not found!");
	}

	bullets = std::vector<Bullet>(64, Bullet(&bulletTexture));
}

void BulletManager::spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float vel, uint8_t playerId, bool isLocal)
{
	Bullet* b = getFirstBullet();
	b->setPosition(pos);
	b->velocity = dir*vel;
	b->setRotation(sf::radians(atan2(-dir.x, dir.y) + 3.14f)); // + pi to rotate due to sprite orientation
	b->resetLifeTime();
	b->firedPlayerId = playerId;
	b->local = isLocal;

	b->isActive = true;
	FMODManager::Instance().playOneshotEvent("gunshot");
}

void BulletManager::clearBullets()
{
	for (auto& b : bullets) {
		b.isActive = false;
	}
}

void BulletManager::update(float dt)
{
	for (auto& b : bullets) {
		if (b.isActive) {
			b.fixedUpdate(dt);
		}
	}
}

void BulletManager::handleLevelCollision(std::vector<sf::FloatRect> platforms)
{
	for (auto& b : bullets) {
		if (!b.isActive) continue;	// skip inactive bullets

		for (auto& p : platforms) {
			if (p.contains(b.getPosition())) {
				b.isActive = false;
				FMODManager::Instance().playOneshotEvent("bulletbreak");
				break;
			}
		}
	}
}

void BulletManager::render(sf::RenderWindow* wn)
{
	for (const auto& b : bullets) {
		if (b.isActive) {
			wn->draw(b);
		}
	}
}

std::vector<Bullet*> BulletManager::getAliveBullets()
{
	std::vector<Bullet*> alive;
	for (auto& b : bullets) {
		if (b.isActive) alive.push_back(&b);
	}
	return alive;
}

Bullet* BulletManager::getFirstBullet(bool active)
{
	for (auto& b : bullets) {
		if (b.isActive == active) return &b;
	}

	// no free bullets so resize vector
	int originalSize = bullets.size();
	bullets.resize(bullets.size() * 2, Bullet(&bulletTexture));
	return &bullets[originalSize];
}
