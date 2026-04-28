#include "Game.h"

Game::Game(sf::RenderWindow* window) 
	: localPlayer(&bulletManager, playerTextures[0], eventsToSend)
{
	game_window = window;

	if (!playerTextures[0].loadFromFile("gfx/player.png")) {
		throw std::invalid_argument("player 1 texture was not found!");
	}
	if (!playerTextures[1].loadFromFile("gfx/player2.png")) {
		throw std::invalid_argument("player 2 texture was not found!");
	}
	if (!playerTextures[2].loadFromFile("gfx/player3.png")) {
		throw std::invalid_argument("player 3 texture was not found!");
	}
	if (!playerTextures[3].loadFromFile("gfx/player4.png")) {
		throw std::invalid_argument("player 4 texture was not found!");
	}

	allPlayers.reserve(4);
	remotePlayers.reserve(3);
	allPlayers.push_back(&localPlayer);
	gameTimeClock.restart();

	std::vector<GameEvent> eventList;

	std::sort(eventList.begin(), eventList.end(), [](GameEvent a, GameEvent b) { return a.type > b.type; });

	Utils::printMsg("");
}

Game::~Game()
{
}

void Game::handleInput(float fixed_timestep, Input* in)
{
	//if (in->left_bumper_down && !testClock.isRunning()) {
	//	testClock.start();
	//}
	//if (!in->left_bumper_down && testClock.isRunning()) {
	//	Utils::printMsg(std::to_string(testClock.reset().asMilliseconds()), debug);
	//}

	if (in->isKeyPressed(sf::Keyboard::Key::P)) {
	}

	if (localPlayer.getHealth()) {
		MovementInputsMessage inputs = localPlayer.handleInput(fixed_timestep, in);
		inputs.time = getGameTime();
		//localPlayer.addInputsMessage(inputs);
	}

	// start match on enter key press
	if (roundNumber == 0) {
		if (in->isKeyPressed(sf::Keyboard::Key::Enter)) {
			Utils::printMsg("Starting match!", success);
			nextRound();
		}
	}

	// debug enemy spawn
	if (in->isKeyPressed(sf::Keyboard::Key::G)) {
		enemyManager.spawnEnemy(&eventsToSend);
	}

	// debug coin spawn
	//if (in->isKeyPressed(sf::Keyboard::Key::C)) {
	//	coinManager.spawnCoin(in->getMousePosWorld(), { 1.0f, 0.0f }, 1);
	//}

	// debug score and money print
	if (in->isKeyPressed(sf::Keyboard::Key::P)) {
		Utils::printMsg(std::format("Time: {}", getGameTime()), debug);
		for (auto& plr : allPlayers) {
			int money = plr->getMoney(); uint8_t points = plr->getPoints();
			Utils::printMsg(std::format("[{}] Money: {}, Points: {}", plr->getId(), plr->getMoney(), plr->getPoints()), debug);
		}
		Utils::printMsg(std::format("state: {}, tospawn: {}", static_cast<int>(getRoundState()), enemiesToSpawn), debug);
	}

	// enable/disable debug printing
	if (in->isKeyPressed(sf::Keyboard::Key::O)) {
		Utils::togglePrints(true);
	}
	if (in->isKeyPressed(sf::Keyboard::Key::X)) {
		Utils::togglePrints(false);
	}
}

void Game::fixedUpdate(float fixed_timestep)
{
	//if (getRoundState() == RoundState::MidRound) {
	//	for (auto& plr : remotePlayers) {
	//		plr.simulateInput(fixed_timestep);
	//	}
	//}

	for (auto& plr : allPlayers) {
		plr->fixedUpdate(fixed_timestep);
		//plr->postCollisionUpdate(fixed_timestep);
	}

	if (getRoundState() == RoundState::PreRound && roundNumber == 1) {
		for (auto& plr : allPlayers) {
			plr->setPosition(level.getSpawnPoint(plr->getId()));
		}
	}

	// process local events
	for (auto& ev : localEvents) {
		if (ev.type & CoinsSpawnRequest) {
			CoinsSpawnRequestMessage csrm = std::get<CoinsSpawnRequestMessage>(ev.var);
			coinManager.processSpawnRequest(csrm, eventsToSend);

			// if the coin was dropped from an enemy, they must have died
			if (csrm.enemy) enemiesSlain++;
		}
		if (ev.type & EnemySpawn) {
			enemiesToSpawn--;
		}
	}
	// erase events we processed here
	// std::erase_if(localEvents, [](GameEvent& ev) {return ev.type & CoinsSpawnRequest;});
	localEvents.clear();

	enemyManager.toggleSpawn(roundNumber != 0 && getRoundState() == RoundState::MidRound && enemiesToSpawn > 0);
	enemyManager.update(fixed_timestep, true, allPlayers, &localEvents, &eventsToSend);

	bulletManager.update(fixed_timestep);
	coinManager.update(fixed_timestep);
	level.update(fixed_timestep);

	//if (NetworkManager::isHost() && getRoundState() == RoundState::PostRound) {
	//	if (roundTimer <= -HOST_DECIDE_WINNER_DELAY.asSeconds() && lastRoundWinner < 0) {
	//		endRound();
	//	}
	//	if (roundTimer <= -HOST_NEW_ROUND_DELAY.asSeconds()) {
	//		if (getPlayerWithId(lastRoundWinner)->getPoints() == POINTS_TO_WIN) {
	//			endMatch();
	//		}
	//		else {
	//			nextRound();
	//		}
	//	}
	//}

	// all enemies slain
	if (getRoundState() == RoundState::PostRound) {
		resetGame();
		nextRound();
	}

	// player died
	if (localPlayer.getHealth() <= 0 && roundNumber != 0) {
		endMatch();
		resetGame(true);
		playerDiedTime = gameTime;
	}

	// revive after 1.5s
	if (gameTime - playerDiedTime > 1.5f && localPlayer.getHealth() == 0) {
		localPlayer.setHealth(3);
		playerDiedTime = -1.0f;
	}

	playerCount = allPlayers.size();
	UIData d;
	d.numPlayers = playerCount; d.roundNo = roundNumber; d.roundTime = roundTimer; d.winningPlayerId = lastRoundWinner;
	d.health = localPlayer.getHealth();
	d.enemiesLeft = roundEnemyCount - enemiesSlain;
	if (roundNumber != 0) {
		d.localPlayerMoney = localPlayer.getMoney();
		d.localPlayerPoints = localPlayer.getPoints();
	}
	uiManager.update(d);

	float realDt = gameTimeClock.restart().asSeconds();
	gameTime += realDt;
	if (roundNumber != 0) {
		roundTimer += realDt;
	}
	//onScreenTimer.setString("Time: " + std::to_string(gameTime));
}

void Game::handleCollisions(float fixed_timestep)
{
	for (auto& plr : allPlayers) {
		//Utils::printMsg(std::format("handle collision, size: {}, plr id: {}, plr addr: {}", allPlayers.size(), plr->getId(), 0));
		plr->handleLevelCollision(level.collisionRects);
		plr->postCollisionUpdate(fixed_timestep);
	}
	for (auto& plr : remotePlayers) {
		plr.remoteUpdate(fixed_timestep);
	}

	bulletManager.handleLevelCollision(level.collisionRects);
	//enemyManager.handleLevelCollision(level.collisionRects); // doesn't do anything rn
	enemyManager.handleBulletCollision(bulletManager.getAliveBullets(), &eventsToSend);

	std::vector<GameEvent>* coinSpawnRequestVector = (NetworkManager::isHost() ? &localEvents : &eventsToSend);
	enemyManager.handlePlayerCollision(localPlayer, coinSpawnRequestVector);

	coinManager.handleLevelCollision(level.collisionRects);
	if (localPlayer.canCollectCoins()) {
		coinManager.handlePlayerCollision(localPlayer, eventsToSend);
	}
}

void Game::networkUpdate(float fixed_timestep, NetworkManager* networkMgr)
{
	// listen for new tcp connections in lobby
	if (networkMgr->isHost() && roundNumber == 0)
		networkMgr->listen(eventsRecieved);

	// create event and state sync packets if ready
	if (networkMgr->eventSendReady()) {
		networkMgr->createEventPackets(eventsToSend);
		if (networkMgr->isHost()) {
			GhostsUpdateMessage msg = enemyManager.createUpdateMessage();
			msg.time_required = networkMgr->eventSendTimer.getElapsedTime().asSeconds();
			networkMgr->createStatePackets(msg);
		}
		networkMgr->eventSendTimer.restart();
	}

	// create input packet if ready
	if (networkMgr->inputSendReady()) {
		PlayerUpdateMessage msg = localPlayer.createUpdateMsg();
		msg.id = networkMgr->getId();
		networkMgr->createInputPacket(msg);
		networkMgr->inputSendTimer.restart();
		localPlayer.clearInputsHistory();
	}

	// send the various types of packets
	// (note that input packets are considered events for this purpose)
	networkMgr->sendEventPackets();
	networkMgr->sendStatePackets();

	// host checks if it needs to start ping chains
	if (networkMgr->isHost())
		networkMgr->startPingChain(getGameTime());

	// load eventsRecieved vector with events from the receieved packets
	networkMgr->receiveEventPackets(eventsRecieved);
	// handle state sync packets (only enemy position updates)
	networkMgr->receiveStatePackets(enemyManager);

	// process each event
	for (auto& event : eventsRecieved) {
		if (event.type & ConnectionChange) {

			// add new player or erase exising player
			ConnectionChangeMessage ccm = std::get<ConnectionChangeMessage>(event.var);
			if (ccm.join) {
				Connection* newConnection = networkMgr->getConnectionWithId(ccm.con.id);
				addPlayer(true, newConnection);
			}
			else {
				removePlayer(ccm.con.id);
			}
		}
		if (event.type & ConnectionList) {
			// update certain vars when we know our id
			ConnectionListMessage clm = std::get<ConnectionListMessage>(event.var);
			localPlayer.setTexture(&playerTextures[clm.receiver_id]);
			localPlayer.setId(clm.receiver_id);
			game_window->setTitle("CMP425 Assessment - PPP - Player " + std::to_string(clm.receiver_id));
		}
		if (event.type & MatchUpdate) {
			MatchUpdateMessage msg = std::get<MatchUpdateMessage>(event.var);
			if (msg.round_start) {
				beginRound(msg);
			}
			else {
				lastRoundWinner = msg.winner;
				getPlayerWithId(msg.winner)->addPoint();
			}
		}
		if (event.type & EnemySpawn) {
			enemyManager.spawnEnemy(std::get<EnemySpawnMessage>(event.var));
		}
		if (event.type & EnemyHurt) {
			enemyManager.hurtEnemy(std::get<EnemyHurtMessage>(event.var));
		}
		if (event.type & CoinsSpawnRequest) {
			coinManager.processSpawnRequest(std::get<CoinsSpawnRequestMessage>(event.var), eventsToSend);
		}
		if (event.type & CoinsSpawn) {
			coinManager.spawnCoins(std::get<CoinsSpawnMessage>(event.var));
		}
		if (event.type & CoinPickup) {
			CoinPickupMessage cpm = std::get<CoinPickupMessage>(event.var);
			coinManager.despawnCoin(cpm.coin_id);
		}
		if (event.type & Ping) {
			PingMessage p = std::get<PingMessage>(event.var);
			setGameTime(p.game_time);
		}
	}
	
	// clear the events now that they're processed
	eventsRecieved.clear();

	// close game if the host disconnects
	if (!networkMgr->isHost() && networkMgr->getConnectionWithId(0) == nullptr) {
		Utils::printMsg("Host disconnected! Closing...", error);
		game_window->close();
	}
}

void Game::addPlayer(bool remote, Connection* connection)
{
	if (remote) {
		uint8_t id = connection->getId();
		Utils::printMsg("Adding player with id: " + std::to_string(id), debug);
		remotePlayers.push_back(Player(&bulletManager, playerTextures[id], eventsToSend, connection));
		allPlayers.push_back(&remotePlayers.back());
		connection->setPlayer(&remotePlayers.back());
	}
	else {
		// we only ever add one local player so this goes unused
	}
}

void Game::removePlayer(uint8_t id)
{
	// auto p = std::erase_if(allPlayers, [id](Player* plr) {return plr->getId() == id; });
	auto num_removed = std::erase_if(remotePlayers, [id](Player& plr) {return plr.getId() == id; });

	// don't need to redo all players vector if erase_if failed
	if (num_removed == 0) return;

	// clear vector, besides first element (local player so can never change)
	while (allPlayers.begin() + 1 != allPlayers.end()) {
		allPlayers.erase(allPlayers.begin() + 1);
	}

	// then add all remote players back into the all players vector
	for (auto& plr : remotePlayers) {
		allPlayers.push_back(&plr);
		plr.getConnection()->setPlayer(&plr);
	}

	Utils::printMsg("erased id " + std::to_string(id), error);
}

void Game::nextRound()
{
	MatchUpdateMessage msg;
	msg.round_num = roundNumber+1;
	msg.round_start = true;
	msg.time = getGameTime() + HOST_MATCH_START_DELAY.asSeconds();
	lastRoundWinner = -1;

	eventsToSend.push_back({ MatchUpdate, msg });
	beginRound(msg);
}

void Game::beginRound(MatchUpdateMessage msg)
{
	for (auto& player : allPlayers) {
		player->setMoney(0);
	}
	lastRoundWinner = -1;
	roundNumber = msg.round_num;
	//roundTimer = ROUND_LENGTH.asSeconds() + ROUND_START_WAIT.asSeconds() + (msg.time - getGameTime());
	roundTimer = 0;
	enemiesToSpawn = 5;
	enemiesSlain = 0;
	Utils::printMsg("Beginning round " + std::to_string(msg.round_num), success);

	resetGame(roundNumber == 0);
}

void Game::endRound()
{
	resetGame(false);

	MatchUpdateMessage msg;
	msg.round_num = roundNumber;
	msg.round_start = false;
	msg.time = getGameTime();

	Player* winningPlayer = getMostWealthyPlayer();
	winningPlayer->addPoint();
	lastRoundWinner = static_cast<int8_t>(winningPlayer->getId());
	msg.winner = lastRoundWinner;
	eventsToSend.push_back({ MatchUpdate, msg });
}

void Game::endMatch()
{
	// back to lobby
	MatchUpdateMessage msg;
	msg.round_num = 0;
	msg.round_start = true;
	msg.time = getGameTime();

	eventsToSend.push_back({ MatchUpdate, msg });
	beginRound(msg);
}

void Game::resetGame(bool clearPoints)
{
	if (clearPoints) {
		for (auto& plr : allPlayers) {
			plr->clearPoints();
		}
	}
	coinManager.clearCoins(roundNumber == 0 ? 0 : 2);
	bulletManager.clearBullets();
	enemyManager.clearEnemies();
}

RoundState Game::getRoundState() const
{
	// always "playing" in the lobby
	if (roundNumber == 0) return RoundState::MidRound;

	// waiting for round to start
	if (roundTimer < ROUND_START_WAIT.asSeconds()) return RoundState::PreRound;

	// round finished, waiting for new one
	if (enemiesSlain == roundEnemyCount) return RoundState::PostRound;

	// default
	return RoundState::MidRound;
}

Player* Game::getMostWealthyPlayer()
{
	auto winnerIt = std::max_element(allPlayers.begin(), allPlayers.end(), [](Player* pl1, Player* pl2) { return pl1->getMoney() < pl2->getMoney();});
	Player* winner = *winnerIt;
	return winner;
}

Player* Game::getPlayerWithId(uint8_t id) const
{
	for (auto& plr : allPlayers) {
		if (plr->getId() == id) {
			return plr;
		}
	}
	return nullptr;
}

void Game::renderGame()
{
	// reset window for next draw
	game_window->clear(sf::Color(254, 153, 0));

	// center window view on local player
	sf::View view = game_window->getView();
	view.setCenter(localPlayer.getWorldCenter());
	game_window->setView(view);

	// draw background
	level.renderBackground(game_window);

	// draw entities
	bulletManager.render(game_window);
	for (auto& plr : allPlayers) {
		if (plr->getHealth()) {
			game_window->draw(*plr);
		}
	}
	level.renderPlatforms(game_window);
	coinManager.render(game_window);
	enemyManager.render(game_window);
	uiManager.render(game_window);

	// present window
	game_window->display();
}
