#pragma once
#include <queue>
#include "SFML/Network.hpp"
#include "Constants.h"

///////////////////////// overloads for default/sfml types /////////////////////////

static sf::Packet& operator<<(sf::Packet& packet, const sf::Vector2f& vec)
{
	return packet << vec.x << vec.y;
}

static sf::Packet& operator>>(sf::Packet& packet, sf::Vector2f& vec)
{
	return packet >> vec.x >> vec.y;
}

enum EventTypes {
	ConnectionChange = 1 << 0,
	ConnectionList = 1 << 1,
	MatchUpdate = 1 << 2,
	EnemySpawn = 1 << 3,
	EnemyHurt = 1 << 4,
	CoinsSpawnRequest = 1 << 5,
	CoinsSpawn = 1 << 6,
	CoinPickup = 1 << 7,

	MovementInputs = 1 << 14,
	Ping = 1 << 15
};

struct MessagePrefix {
	std::uint16_t events;
};

/////////////////// playerconnection ///////////////////

struct PlayerConnection {
	uint32_t ip;
	uint16_t port;
	uint8_t id;
	std::string name;
};

static sf::Packet& operator<<(sf::Packet& packet, const PlayerConnection& pc) {
	return packet << pc.ip << pc.port << pc.id << pc.name;
}

static sf::Packet& operator>>(sf::Packet& packet, PlayerConnection& pc) {
	return packet >> pc.ip >> pc.port >> pc.id >> pc.name;
}

struct ConnectionChangeMessage {
	PlayerConnection con;
	bool join;
};

struct ConnectionListMessage {
	uint8_t receiver_id;
	uint8_t num;
	PlayerConnection con[2];
};

////////////////// PingMessage //////////////////

struct PingMessage {
	uint8_t ping_num;
	uint8_t receiver_id;
	float latency;
	float game_time;
};

static sf::Packet& operator<<(sf::Packet& packet, const PingMessage& ping) {
	return packet << ping.ping_num << ping.receiver_id << ping.latency << ping.game_time;
}

static sf::Packet& operator>>(sf::Packet& packet, PingMessage& ping) {
	return packet >> ping.ping_num >> ping.receiver_id >> ping.latency >> ping.game_time;
}

////////////////// MatchUpdateMessage //////////////////

struct MatchUpdateMessage {
	bool round_start;
	uint8_t round_num;
	int8_t winner = -1; // -1 = no winner
	float time;
};

static sf::Packet& operator<<(sf::Packet& packet, const MatchUpdateMessage& mum) {
	return packet << mum.round_start << mum.round_num << mum.winner << mum.time;
}

static sf::Packet& operator>>(sf::Packet& packet, MatchUpdateMessage& mum) {
	return packet >> mum.round_start >> mum.round_num >> mum.winner >> mum.time;
}

////////////////// EnemySpawnMessage //////////////////

struct EnemySpawnMessage {
	uint8_t enemy_id;
	uint16_t seed;
};

static sf::Packet& operator<<(sf::Packet& packet, const EnemySpawnMessage& esm)
{
	return packet << esm.enemy_id << esm.seed;
}

static sf::Packet& operator>>(sf::Packet& packet, EnemySpawnMessage& esm)
{
	return packet >> esm.enemy_id >> esm.seed;
}

////////////////// EnemyHurtMessage //////////////////

struct EnemyHurtMessage {
	uint8_t enemy_id;
	uint16_t damage_taken;
	uint8_t attacking_player_id;

	EnemyHurtMessage(uint8_t enemyId, uint16_t damage, uint8_t playerId) {
		enemy_id = enemyId;
		damage_taken = damage;
		attacking_player_id = playerId;
	}
	EnemyHurtMessage() {};
};

static sf::Packet& operator<<(sf::Packet& packet, const EnemyHurtMessage& ehm)
{
	return packet << ehm.enemy_id << ehm.damage_taken << ehm.attacking_player_id;
}

static sf::Packet& operator>>(sf::Packet& packet, EnemyHurtMessage& ehm)
{
	return packet >> ehm.enemy_id >> ehm.damage_taken >> ehm.attacking_player_id;
}

////////////////// CoinsSpawnRequestMessage //////////////////

struct CoinsSpawnRequestMessage {
	uint8_t num_coins;
	uint16_t total_worth;
	sf::Vector2f position;
};

static sf::Packet& operator<<(sf::Packet& packet, const CoinsSpawnRequestMessage& csrm)
{
	return packet << csrm.num_coins << csrm.total_worth << csrm.position;
}

static sf::Packet& operator>>(sf::Packet& packet, CoinsSpawnRequestMessage& csrm)
{
	return packet >> csrm.num_coins >> csrm.total_worth >> csrm.position;
}

////////////////// CoinsSpawnMessage //////////////////

struct CoinsSpawnMessage {
	uint8_t num_coins;
	std::array<uint8_t, MAX_COINS_SPAWN> coin_ids;
	uint16_t total_worth;
	sf::Vector2f position;
	uint16_t seed;
};

static sf::Packet& operator<<(sf::Packet& packet, const CoinsSpawnMessage& csm)
{
	packet << csm.num_coins << csm.total_worth << csm.position << csm.seed;
	for (uint8_t i = 0; i < csm.num_coins; i++) {
		packet << csm.coin_ids[i];
	}
	return packet;
}

static sf::Packet& operator>>(sf::Packet& packet, CoinsSpawnMessage& csm)
{
	packet >> csm.num_coins >> csm.total_worth >> csm.position >> csm.seed;
	for (uint8_t i = 0; i < csm.num_coins; i++) {
		packet >> csm.coin_ids[i];
	}
	return packet;
}

////////////////// CoinPickupMessage //////////////////

struct CoinPickupMessage {
	uint8_t coin_id;
	// float time ?

	CoinPickupMessage(uint8_t id)
		: coin_id{ id } { }

	CoinPickupMessage() {}
};

static sf::Packet& operator<<(sf::Packet& packet, const CoinPickupMessage& cpm) {
	return packet << cpm.coin_id;
}

static sf::Packet& operator>>(sf::Packet& packet, CoinPickupMessage& cpm) {
	return packet >> cpm.coin_id;
}

////////////////// EVENTS DEFINITION //////////////////

// https://en.cppreference.com/w/cpp/utility/variant/visit
using EventVariant = std::variant<
	ConnectionChangeMessage,
	ConnectionListMessage,
	MatchUpdateMessage,
	PingMessage,
	EnemySpawnMessage,
	EnemyHurtMessage,
	CoinsSpawnRequestMessage,
	CoinsSpawnMessage,
	CoinPickupMessage
>;

struct GameEvent {
	EventTypes type;
	EventVariant var;
};

////////////////// player update message /////////////

enum MovementInputFlags {
	JumpFlag = 1 << 1,
	FiredFlag = 1 << 2,
	HurtFlag = 1 << 3,
	HurtLeftFlag = 1 << 4,
	ThrottleFlag = 1 << 5,
	ThrottleLeftFlag = 1 << 6
};

struct MovementInputsMessage {
	uint8_t movementFlags = 0;
	float fire_angle = 0.0f;
	float time;

	// for local processing only!
	std::optional<sf::Vector2f> position_after;
};

static sf::Packet& operator<<(sf::Packet& packet, const MovementInputsMessage& mim)
{
	packet << mim.movementFlags << mim.time;
	if (mim.movementFlags & FiredFlag) { packet << mim.fire_angle; }
	return packet;
}

static sf::Packet& operator>>(sf::Packet& packet, MovementInputsMessage& mim)
{
	packet >> mim.movementFlags >> mim.time;
	if (mim.movementFlags & FiredFlag) { packet >> mim.fire_angle; }
	return packet;
}

struct PlayerUpdateMessage {
	uint8_t id;
	uint16_t money;
	sf::Vector2f position;
	sf::Vector2f velocity;
	uint8_t inputs_size = 0;
	MovementInputsMessage inputs[6];

	uint8_t _local_inputs_processed = 0;
};

static sf::Packet& operator<<(sf::Packet& packet, const PlayerUpdateMessage& pum)
{
	packet << pum.id << pum.money << pum.position << pum.velocity << pum.inputs_size;
	for (uint8_t i = 0; i < pum.inputs_size; i++) {
		packet << pum.inputs[i];
	}
	return packet;
}

static sf::Packet& operator>>(sf::Packet& packet, PlayerUpdateMessage& pum)
{
	packet >> pum.id >> pum.money >> pum.position >> pum.velocity >> pum.inputs_size;
	for (uint8_t i = 0; i < pum.inputs_size; i++) {
		packet >> pum.inputs[i];
	}
	return packet;
}

////////////////// GhostsUpdateMessage /////////////

struct GhostUpdate {
	uint8_t ghost_id;
	sf::Vector2f position;

	GhostUpdate(uint8_t id, sf::Vector2f pos) {
		ghost_id = id;
		position = pos;
	}
	GhostUpdate() {};
};

static sf::Packet& operator<<(sf::Packet& packet, const GhostUpdate& gm)
{
	return packet << gm.ghost_id << gm.position;
}

static sf::Packet& operator>>(sf::Packet& packet, GhostUpdate& gm)
{
	return packet >> gm.ghost_id >> gm.position;
}

struct GhostsUpdateMessage {
	uint16_t packet_id = 0;
	uint8_t num_ghosts = 0;
	float time_required = -1.0f;
	std::array<GhostUpdate, MAX_NUM_ENEMY> updates;
};

static sf::Packet& operator<<(sf::Packet& packet, const GhostsUpdateMessage& gum)
{
	packet << gum.packet_id << gum.num_ghosts << gum.time_required;
	for (int i = 0; i < gum.num_ghosts; i++) {
		packet << gum.updates[i];
	}
	return packet;
}

static sf::Packet& operator>>(sf::Packet& packet, GhostsUpdateMessage& gum)
{
	packet >> gum.packet_id >> gum.num_ghosts >> gum.time_required;
	for (int i = 0; i < gum.num_ghosts; i++) {
		packet >> gum.updates[i];
	}
	return packet;
}
