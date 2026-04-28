#include "Player.h"

Player::Player(BulletManager* bm, sf::Texture& tex, std::vector<GameEvent>& events, Connection* con)
	: eventsToSendList(&events)
{
	bulletManager = bm;
	connection = con;

	if (con != nullptr) {
		id = con->getId();
	}

	setTexture(&tex);
		
	setSize(sf::Vector2f(34, 34));
	setOrigin(sf::Vector2f());
	setCollisionRect(sf::FloatRect({ 2, 8 }, { 26, 26 }));
	setPosition({ 100, 50 });

	walk.addFrame(sf::IntRect({ 4, 4 }, { 17, 17 }));
	walk.addFrame(sf::IntRect({ 25, 4 }, { 17, 17 }));
	walk.addFrame(sf::IntRect({ 4, 4 }, { 17, 17 }));
	walk.addFrame(sf::IntRect({ 46, 4 }, { 17, 17 }));
	walk.addFrame(sf::IntRect({ 4, 4 }, { 17, 17 }));
	walk.setAnimationSpeed(1.f / 6.f);
	walk.setFrameSpeed(0, 5.0f);

	jump.addFrame(sf::IntRect({ 4, 25 }, { 17, 17 }));
	fall.addFrame(sf::IntRect({ 25, 25 }, { 17, 17 }));
	hurt.addFrame(sf::IntRect({ 45, 25 }, { 17, 17 }));

	currentAnim = &walk;
}

MovementInputsMessage Player::handleInput(float dt, Input* in)
{
	MovementInputsMessage inputsMsg;

	if ((in->isKeyPressed(sf::Keyboard::Key::Space)) && (coyoteTimer > 0.0f || airJumpsRemaining > 0)) {
		velocity.y = -3.0f * SCALE;
		if (coyoteTimer <= 0.0f) {
			airJumpsRemaining = airJumpsRemaining - 1;
		}
		inputsMsg.movementFlags |= JumpFlag;

		FMODManager::Instance().playOneshotEvent("MainMusic");
	}

	throttle = 0.0f;
	if (!isStunned()) {
		if (in->isKeyDown(sf::Keyboard::Key::A)) {
			throttle -= 1.0f;
		}
		if (in->isKeyDown(sf::Keyboard::Key::D)) {
			throttle += 1.0f;
		}

		if (in->isMouseButtonPressed(sf::Mouse::Button::Left)
		|| (in->isMouseButtonDown(sf::Mouse::Button::Left) && timeSinceFired > rateOfFire)) {
			timeSinceFired = 0.0f;
			sf::Vector2f pos;
			pos = getPosition() + (facingRight ? RIGHT_BULLET_OFFSET : LEFT_BULLET_OFFSET);
			sf::Vector2f dir = (in->getMousePosWorld() - pos).normalized();

			bulletManager->spawnBullet(pos, dir, BULLET_SPAWN_VELOCITY, getId(), true);

			inputsMsg.movementFlags |= FiredFlag;
			inputsMsg.fire_angle = dir.angle().asRadians();
		}
	}

	if (throttle != 0.0f) {
		inputsMsg.movementFlags |= ThrottleFlag;
		if (throttle < 0.0f) inputsMsg.movementFlags |= ThrottleLeftFlag;
	}

	return inputsMsg;

	// debug flight
	//if (in->isKeyDown(sf::Keyboard::Key::W)) {
	//	velocity.y = -150.0f;
	//}
	//else if (in->isKeyDown(sf::Keyboard::Key::S)) {
	//	velocity.y = 150.0f;
	//}
	//else {
	//	velocity.y = 0.0f;
	//}

	// debug mouse attack
	//if (in->isMouseButtonPressed(sf::Mouse::Button::Middle)) {
	//	takeDamage(in->mouse_pos_local.x, nullptr);
	//}

	//// debug reset
	//if (in->isKeyDown(sf::Keyboard::Key::R)) {
	//	velocity = { 0,0 };
	//	setPosition({ 50, 50 });
	//}
}

// network function
void Player::simulateInput(float dt)
{
	// can't simulate no inputs!
	if (messageHistory.empty()) return;

	// get the message we should be processing
	PlayerUpdateMessage& msg = messageHistory.front();
	MovementInputsMessage in = msg.inputs[msg._local_inputs_processed];

	// using the message's contents, simulate user input
	if (in.movementFlags & JumpFlag) {
		velocity.y = -3.0f * SCALE;
	}

	if (in.movementFlags & ThrottleFlag) {
		throttle = (in.movementFlags & ThrottleLeftFlag ? -1.0f : 1.0f);
	}
	else {
		throttle = 0;
	}

	if (in.movementFlags & FiredFlag) {
		sf::Vector2f dir = sf::Vector2f(1.0f, sf::radians(in.fire_angle));
		sf::Vector2f pos = getPosition() + (facingRight ? RIGHT_BULLET_OFFSET : LEFT_BULLET_OFFSET);
		bulletManager->spawnBullet(pos, dir, BULLET_SPAWN_VELOCITY, getId(), false);
	}

	if (in.movementFlags & HurtFlag) {
		float attackX = (in.movementFlags & HurtLeftFlag ? getPosition().x - 10.0f : getPosition().x + 10.0f);
		takeDamage(attackX, nullptr);
	}

	msg._local_inputs_processed = msg._local_inputs_processed + 1;
	//Utils::printMsg("throttle: " + std::to_string(throttle) + ", jump: " + std::to_string(in.jump));
}

void Player::fixedUpdate(float dt)
{
	// update the y velocity by the gravity value
	velocity.y += GRAVITY * SCALE * dt;

	// accelerate the player if walking
	if (throttle != 0.0f) {
		float bonusAccel = 1.0f;
		if ((throttle > 0.0f && velocity.x < 0.0f) || (throttle < 0.0f && velocity.x > 0.0f)) {
			bonusAccel = 3.0f;
		}
		velocity.x += ACCEL * SCALE * throttle * bonusAccel * dt;

		facingRight = (throttle > 0.0f);
	}
	// decelerate when not walking
	else {
		velocity.x = velocity.x / 1.08f;
		if (isGrounded) {
			velocity.x = velocity.x / 1.25f;
		}
	}

	// cap velocity values
	if (velocity.y > MAX_FALL_SPEED) velocity.y = MAX_FALL_SPEED;

	if (velocity.x > MAX_HORIZONTAL_SPEED) velocity.x = MAX_HORIZONTAL_SPEED;
	else if (velocity.x < -MAX_HORIZONTAL_SPEED) velocity.x = -MAX_HORIZONTAL_SPEED;

	coyoteTimer -= dt;
	timeSinceHit += dt;
	timeSinceFired += dt;

	// move the player by the current velocity
	move(velocity * dt);

	// set grounded to false before next collision check
	setGrounded(false);

	// handle flashing for iframes
	if (isImmune() && !isStunned()) {
		int timeSinceHitInt = static_cast<int>(std::trunc(timeSinceHit * 10.0f));
		if (timeSinceHitInt % 2 == 0) {
			setFillColor({ 255, 255, 255, 75 });
		}
		else {
			setFillColor({ 255, 255, 255, 255 });
		}
	}
	else setFillColor({ 255, 255, 255, 255 });
}

void Player::remoteUpdate(float dt)
{
	if (messageHistory.size() == 0) return;
	PlayerUpdateMessage msg = messageHistory.front();

	// if we've processed every local input, we can compare positions
	if (msg._local_inputs_processed == msg.inputs_size) {
		float distanceSquared = sf::Vector2f(msg.position - getPosition()).lengthSquared();

		// if the positions are too far apart, adjust position and simulation accordingly
		if (distanceSquared > PLAYER_DESYNC_DISTANCE_SQUARED) {
			Utils::printMsg("Desync spotted, teleporting...", warning);
			setPosition(msg.position);
			velocity = msg.velocity;
		}
		// since we have no more inputs to process, we can discard this message
		messageHistory.pop_front();
	}
}

void Player::addCollider(sf::FloatRect rect)
{
	// if we can't fit this in the vector, push it back to make space
	if (collidingRects.size() < numColliders + 1) {
		collidingRects.push_back(rect);
	}
	// otherwise write this in place over what was there before
	else {
		collidingRects[numColliders] = rect;
	}
	numColliders++;
}

// https://c.har.li/e/2024/03/28/implementing-robust-2D-collision-resolution.htm
void Player::handleLevelCollision(std::vector<sf::FloatRect> platforms)
{
	for (auto& p : platforms) {
		if (p.findIntersection(getCollisionRect()).has_value()) {
			addCollider(p);
		}
	}

	if (numColliders == 0) return;

	// run twice to account for corners, unless only one collider
	int iter = (numColliders == 1 ? 1 : 2);
	for (int i = 0; i < iter; i++) {
		auto end = collidingRects.begin() + (numColliders);
		sf::FloatRect thisColRect = getCollisionRect();

		// get the largest collision rectangle by area
		// todo: probably optimisable by calcing intersect rects once and storing
		sf::FloatRect mostOverlap;
		if (numColliders > 1) {
			auto result = std::max_element(collidingRects.begin(), end, [thisColRect](sf::FloatRect a, sf::FloatRect b) {
				sf::FloatRect ca = thisColRect.findIntersection(a).value();
				sf::FloatRect cb = thisColRect.findIntersection(b).value();
				return (ca.size.x * ca.size.y) < (cb.size.x * cb.size.y);
				});
			mostOverlap = thisColRect.findIntersection(*result).value();

			// need to erase this rect since we wont be colliding with it anymore
			collidingRects.erase(result);
			numColliders--;
		}
		else {
			auto opt = thisColRect.findIntersection(collidingRects[0]);
			if (!opt.has_value()) break;

			mostOverlap = opt.value();
		}

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
				setGrounded(true);
			}
			else { // moving up
				move({ 0.0f, mostOverlap.size.y });
			}
			velocity.y = 0.0f;
		}
	}

	// once we're done, set the number of colliders to 0
	numColliders = 0;
	return;
}

void Player::postCollisionUpdate(float dt)
{
	decideAnimationState();

	currentAnim->animate(dt);
	currentAnim->setHorizontalFlip(facingRight);
	setTextureRect(currentAnim->getCurrentFrame());

	throttle = 0; // so if we don't process inputs, we won't keep trying to move
}

void Player::takeDamage(float attackX, std::vector<GameEvent>* coinSpawnRequestList, bool addFlag, float knockback, bool ignoreStun)
{
	if (timeSinceHit < IFRAME_TIME && !ignoreStun) return;
	timeSinceHit = 0.0f;
	health--;
	// aud->playSound("hurt");
	
	// calculate direction and push player that way, and a little bit up
	float dir = attackX > getPosition().x + getSize().x ? -1.0f : 1.0f;
	velocity = { 1.0f * SCALE * knockback * dir, std::min(velocity.y, -1.5f * SCALE) };
	facingRight = (dir < 0.0f ? true : false);

	if (addFlag && !movementInputsHistory.empty()) {
		movementInputsHistory.back().movementFlags |= HurtFlag;
		if (dir < 0.0f) {
			movementInputsHistory.back().movementFlags |= HurtLeftFlag;
		}
	}

	setGrounded(false);
	coyoteTimer = 0.0f;

	// request the spawning of coins, if this was a local player that took damage
	if (coinSpawnRequestList == nullptr) return;
	CoinsSpawnRequestMessage csrm;
	csrm.num_coins = (rand() % 5) + 1;
	csrm.total_worth = money / 2;
	csrm.position = getPosition();
	coinSpawnRequestList->push_back({ CoinsSpawnRequest, csrm });

	money -= csrm.total_worth;
}

PlayerUpdateMessage Player::createUpdateMsg() const
{
	PlayerUpdateMessage msg;
	msg.id = id;
	msg.money = getMoney();
	msg.position = getPosition();
	msg.velocity = velocity;
	msg.inputs_size = movementInputsHistory.size();
	for (int i = 0; i < msg.inputs_size; i++) {
		msg.inputs[i] = movementInputsHistory[i];
	}

	return msg;
}

void Player::addUpdateMessage(PlayerUpdateMessage msg)
{
	messageHistory.push_back(msg);
	setMoney(msg.money);

	if (messageHistory.size() > MAX_MESSAGE_HISTORY_SIZE) {
		//Utils::printMsg(std::format("({}) max message history size reached, erasing prematurely (desync likely)", getId()), warning);
		messageHistory.pop_front();
	}
}

void Player::addInputsMessage(MovementInputsMessage msg)
{
	movementInputsHistory.push_back(msg);

	if (movementInputsHistory.size() > MAX_INPUT_HISTORY_SIZE) {
		//Utils::printMsg(std::format("({}) max movementInputHistory size reached, erasing prematurely (desync likely)", getId()), warning);
		movementInputsHistory.pop_front();
	}
}

void Player::decideAnimationState()
{
	if (isStunned()) {
		currentAnim = &hurt;
		setFillColor({ 255, 0, 0 });
		return;
	}

	if (!isGrounded) {
		if (velocity.y > -1.0f) {
			currentAnim = &fall;
		}
		else {
			currentAnim = &jump;
		}
		return;
	}

	currentAnim = &walk;
	if (throttle == 0.0f) {
		currentAnim->reset();
		currentAnim->pause();
	}
	else {
		currentAnim->play();
	}
}

void Player::setGrounded(bool grounded)
{
	isGrounded = grounded;
	if (grounded) {
		coyoteTimer = COYOTE_TIME;
		airJumpsRemaining = MAX_AIR_JUMPS;
	}
}
