#include "UI.h"

UI::UI()
	: topMiddleText(font), localPlayerInfoText(font), debugGameTimer(font), scoreText(font)
{
	if (!font.openFromFile("font/pmscript.ttf")) {
		Utils::printMsg("couldn't load font!", error);
	}

	debugGameTimer.setOutlineThickness(2.0f);
	debugGameTimer.setOutlineColor(sf::Color::Black);

	topMiddleText.setOutlineThickness(2.0f);
	topMiddleText.setOutlineColor(sf::Color::Black);
	topMiddleText.setString("Press enter to start...");

	localPlayerInfoText.setOutlineThickness(1.0f);
	localPlayerInfoText.setOutlineColor(sf::Color::Black);
	localPlayerInfoText.setCharacterSize(22);

	scoreText.setOutlineThickness(2.0f);
	scoreText.setOutlineColor(sf::Color::Black);
	scoreText.setCharacterSize(22);
	scoreText.setString("I'm normal! Hahahah! Eggs!!");
}

void UI::update(UIData data)
{
	if (data.localPlayerMoney >= 0) {
		std::string t;
		t = std::format("Money: ${}", data.localPlayerMoney);
		if (data.localPlayerPoints >= 0) {
			t.append(std::format(" / Health: {}", data.health));
		}
		localPlayerInfoText.setString(t);
	}
	else localPlayerInfoText.setString("");

	if (data.winningPlayerId != -1) {
		std::string t = std::format("Wave {} clear!", data.roundNo);
		topMiddleText.setString(t);
		return;
	}

	// in game
	if (data.roundNo != 0) {
		std::string t;
		if (data.roundTime < ROUND_START_WAIT.asSeconds()) {
			t = std::format("Wave {} starting in {}...", data.roundNo, ceil(ROUND_START_WAIT.asSeconds() - data.roundTime));
		}
		else if (data.roundTime < 0) {
			t = "Finished!";
		}
		else {
			t = std::format("Wave {} / {} enemies left!", data.roundNo, data.enemiesLeft);
		}
		topMiddleText.setString(t);
		return;
	}

	// lobby
	else {
		if (data.health == 0) { topMiddleText.setString("GAME OVER!"); }
		else { topMiddleText.setString("Press enter to start..."); }
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

	scoreText.setPosition(view.getCenter() + sf::Vector2f(-5.0f + (win->getSize().x / 2.0f) - scoreText.getLocalBounds().size.x, (5.0f - (win->getSize().y / 2.0f))));
	win->draw(scoreText);
}
