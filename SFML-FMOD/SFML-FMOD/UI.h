#pragma once
#include "SFML/Graphics.hpp";
#include "NetworkManager.h"
#include <format>

struct UIData {
	int numPlayers = 1;
	int roundNo = 0;
	float roundTime = 0.0f;
	int8_t winningPlayerId = -1;
	int localPlayerMoney = -1;
	int localPlayerPoints = -1;
};

class UI
{
public:
	UI();
	void update(UIData data);
	void render(sf::RenderWindow* win);

private:
	sf::Font font;					// the font for everything

	sf::Text topMiddleText;			// where round time and "waiting"/"ready" goes
	sf::Text localPlayerInfoText;	// displays money and wins

	int playerCount = 1;

	sf::Text debugGameTimer;		// displays the total time elapsed
};

