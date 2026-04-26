#include "UI.h"

UI::UI()
	: topMiddleText(font), localPlayerInfoText(font), debugGameTimer(font)
{
	if (!font.openFromFile("font/pmscript.ttf")) {
		Utils::printMsg("couldn't load font!", error);
	}

	debugGameTimer.setOutlineThickness(2.0f);
	debugGameTimer.setOutlineColor(sf::Color::Black);

	topMiddleText.setOutlineThickness(2.0f);
	topMiddleText.setOutlineColor(sf::Color::Black);
	topMiddleText.setString("Waiting for players... (1/4)");

	localPlayerInfoText.setOutlineThickness(1.0f);
	localPlayerInfoText.setOutlineColor(sf::Color::Black);
	localPlayerInfoText.setCharacterSize(22);
}

void UI::update(UIData data)
{
	if (data.localPlayerMoney >= 0) {
		std::string t;
		t = std::format("Money: ${}", data.localPlayerMoney);
		if (data.localPlayerPoints >= 0) {
			t.append(std::format(" / Points: {}", data.localPlayerPoints));
		}
		localPlayerInfoText.setString(t);
	}
	else localPlayerInfoText.setString("");

	if (data.winningPlayerId != -1) {
		std::string t = std::format("Player {} wins round {}!", data.winningPlayerId, data.roundNo);
		topMiddleText.setString(t);
		return;
	}

	// in game
	if (data.roundNo != 0) {
		std::string t;
		if (data.roundTime > ROUND_LENGTH.asSeconds()) {
			t = std::format("Round {} starting in {}...", data.roundNo, ceil(data.roundTime - ROUND_LENGTH.asSeconds()));
		}
		else if (data.roundTime < 0) {
			t = "Finished!";
		}
		else {
			t = std::to_string(data.roundTime);
		}
		topMiddleText.setString(t);
		return;
	}

	// in lobby
	// todo: setting text every frame (evil)
	if (data.numPlayers != playerCount) {
		playerCount = data.numPlayers;
	}
	if (playerCount == 1) {
		topMiddleText.setString("Waiting for players... (1/4)");
	}
	else {
		if (NetworkManager::isHost()) {
			std::string t = std::format("Press Enter to start! ({}/4)", playerCount);
			topMiddleText.setString(t);
		}
		else {
			std::string t = std::format("Waiting for host to start... ({}/4)", playerCount);
			topMiddleText.setString(t);
		}
	}
}

void UI::render(sf::RenderWindow* win)
{
	// draw ui
	sf::View view = win->getView();

	topMiddleText.setPosition(view.getCenter() + sf::Vector2f(-(topMiddleText.getLocalBounds().size.x/2.0f), (5.0f - (win->getSize().y / 2.0f))));
	win->draw(topMiddleText);

	localPlayerInfoText.setPosition(view.getCenter() + sf::Vector2f(5.0f - (win->getSize().x / 2.0f), (5.0f - (win->getSize().y / 2.0f))));
	win->draw(localPlayerInfoText);
}
