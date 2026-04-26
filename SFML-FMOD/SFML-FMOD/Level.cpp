#include "Level.h"

Level::Level()
	: levelSprite(levelTexture)
{
	if (!skyTexture.loadFromFile("gfx/nszsky.png")) {
		Utils::printMsg("Couldn't load sky texture!", error);
	}
	if (!rockTexture.loadFromFile("gfx/nszrock.png")) {
		Utils::printMsg("Couldn't load rock texture!", error);
	}
	if (!seaTexture0.loadFromFile("gfx/nszsea0.png")) {
		Utils::printMsg("Couldn't load sea 0 texture!", error);
	}
	if (!seaTexture1.loadFromFile("gfx/nszsea1.png")) {
		Utils::printMsg("Couldn't load sea 1 texture!", error);
	}
	if (!levelTexture.loadFromFile("gfx/levelTiles.png")) {
		Utils::printMsg("Couldn't load tiles!", error);
	}


	////////////// initialise platforms /////////////////////

	// level bounds = 24x22
	// first 4 are bounding box
	// remaining are top to bottom, left to right
	platformData = { {{0,0},{24,1}},
		{{0,1},{1,20}},
		{{23, 1},{1,20}},
		{{0,21},{24,1}},

		{{11,6},{2,1}},
		{{2,7}, {3,1}},
		{{19,7},{3,1}},
		{{8,9},{1,1}},
		{{15,9},{1,1}},
		{{4,11},{3,1}},
		{{17,11},{3,1}},
		{{11,12},{2,1}},
		{{1,14},{1,1}},
		{{22,14},{1,1}},
		{{10,15},{4,1}},
		{{5,17},{2,1}},
		{{17,17},{2,1}},
		{{3,18},{6,1}},
		{{15,18},{6,1}},
		{{11,20},{2,1}}
	};

	Entity p;
	p.setFillColor(sf::Color::White);
	const float scale = 42.0f;

	for (auto data : platformData) {
		p.setSize(data.size * scale);
		p.setPosition(data.pos * scale);
		p.setCollisionRect(sf::FloatRect({}, p.getSize()));
		platforms.push_back(p);
	}

	for (auto& p : platforms) {
		collisionRects.push_back(p.getCollisionRect());
	}

	////////////// initialise background //////////////////

	skyShape.setTexture(&skyTexture);
	rockShape.setTexture(&rockTexture);
	seaShape0.setTexture(&seaTexture0);
	seaShape1.setTexture(&seaTexture1);

	skyShape.setSize({ 1536, 3072 });
	skyShape.setOrigin({ 768, 1536 });
	rockShape.setSize({ 1536, 216 });
	rockShape.setOrigin({ 768, 135 });
	seaShape0.setSize({ 1536, 27 });
	seaShape0.setOrigin({ 768, -81 });
	seaShape1.setSize({ 1536, 336 });
	seaShape1.setOrigin({ 768, -102 });

	rockAnim.addFrame({ {0,0}, {416,72} });
	rockAnim.addFrame({ {0,72}, {416,72} });
	rockAnim.setAnimationSpeed(0.75f);

	levelShader.loadFromFile("shaders/level_shader.glsl", sf::Shader::Type::Fragment);
	levelSprite.setPosition({});
	levelSprite.setTexture(levelTexture, true);
	levelShader.setUniform("levelTexture", levelTexture);
}

void Level::update(float dt)
{
	rockAnim.animate(dt);
	rockShape.setTextureRect(rockAnim.getCurrentFrame());
}

void Level::renderPlatforms(sf::RenderWindow* win)
{
	//for (auto& p : platforms) {
	//	win->draw(p);
	//}

	levelShader.setUniform("screenBottom", int(win->getSize().y));
	win->draw(levelSprite, &levelShader);
	//win->draw(levelSprite);
}

void Level::renderBackground(sf::RenderWindow* win)
{
	sf::View v = win->getView();
	sf::Vector2f c = v.getCenter();

	skyShape.setPosition(c - sf::Vector2f(c.x / 20.0f, c.y / 50.0f));
	rockShape.setPosition(c - sf::Vector2f(c.x / 12.0f, c.y / 50.0f));
	seaShape0.setPosition(c - sf::Vector2f(c.x / 10.0f, c.y / 50.0f));
	seaShape1.setPosition(c - sf::Vector2f(c.x / 8.0f, c.y / 50.0f));

	win->draw(skyShape);
	win->draw(rockShape);
	win->draw(seaShape0);
	win->draw(seaShape1);
}

sf::Vector2f Level::getSpawnPoint(uint8_t id)
{
	if (id >= 0 && id < MAX_PLAYERS) {
		return spawnPoints[id];
	}
	return sf::Vector2f();
}
