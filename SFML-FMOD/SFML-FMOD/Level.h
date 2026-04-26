#pragma once
#include <vector>
#include "SFML Framework/CollisionEntity.h"
#include "SFML Framework/Animation.h"
#include "utils.h"
#include "Constants.h"

struct PlatformData {
	sf::Vector2f pos;
	sf::Vector2f size;

	PlatformData(sf::Vector2f p, sf::Vector2f s) {
		pos = p;
		size = s;
	}
};

class Level
{
public:
	Level();
	~Level() {};

	void update(float dt);
	void renderPlatforms(sf::RenderWindow* win);
	void renderBackground(sf::RenderWindow* win);

	sf::Vector2f getSpawnPoint(uint8_t id);

	// todo: what is this encapsulation
	std::vector<Entity> platforms;
	std::vector<sf::FloatRect> collisionRects;

	//sf::Texture& getTexture() { return levelTexture; }

private:
	std::vector<PlatformData> platformData;
	std::array<sf::Vector2f, MAX_PLAYERS> spawnPoints = {
		sf::Vector2f(210.0f, 428.0f),
		sf::Vector2f(714.0f, 428.0f),
		sf::Vector2f(210.0f, 680.0f),
		sf::Vector2f(714.0f, 680.0f)
	};

	sf::RectangleShape skyShape;
	sf::RectangleShape rockShape;
	sf::RectangleShape seaShape0;
	sf::RectangleShape seaShape1;

	sf::Texture skyTexture;
	sf::Texture rockTexture;
	sf::Texture seaTexture0;
	sf::Texture seaTexture1;
	Animation rockAnim;

	sf::Texture levelTexture;
	sf::Shader levelShader;
	sf::Sprite levelSprite;
};

