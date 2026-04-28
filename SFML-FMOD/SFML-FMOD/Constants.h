#pragma once
#include <cstdint>

////////////// physics/world values
inline static constexpr float SCALE = 200.0f;
inline static constexpr float GRAVITY = 9.8f;
inline static constexpr float MAX_FALL_SPEED = SCALE * 4.0f;
inline static constexpr float COIN_SPAWN_VELOCITY = 1.5f * SCALE;
inline static constexpr float BULLET_SPAWN_VELOCITY = 4.0f * SCALE;

////////////// entity max counts
inline static constexpr uint8_t MAX_PLAYERS = 4;
inline static constexpr uint8_t MAX_NUM_ENEMY = 12;
inline static constexpr uint8_t MAX_NUM_COINS = 128;

////////////// gameplay
inline static constexpr sf::Time ROUND_LENGTH = sf::seconds(16);
inline static constexpr sf::Time ROUND_START_WAIT = sf::seconds(3);
inline static constexpr int POINTS_TO_WIN = 2;

///////////// player
inline static constexpr float BASE_FIRE_RATE = 0.33f;

// the maximum number of coins that can be spawned from any source at once
inline static constexpr uint8_t MAX_COINS_SPAWN = 8;

// the aggro applied by any attack, before considering its damage
inline static constexpr float BASE_ATTACK_AGGRO = 10.0f;

////////////// networking 
// squared distance players can be from their predicted position before a recalculation is necessary
inline static constexpr float PLAYER_DESYNC_DISTANCE_SQUARED = 100.0f;

// todo: ?
static inline constexpr float ENEMY_DESYNC_DISTANCE_SQUARED = 36.0f;

// how often to send event packets and player input packets
inline static constexpr sf::Time EVENT_SEND_RATE = sf::seconds(1.0f / 15.0f); // max 15 packets/sec
inline static constexpr sf::Time INPUT_SEND_RATE = sf::seconds(1.0f / 20.0f); // 20 packets/sec, could be lower

// how often to start ping chains
inline static constexpr sf::Time PING_WAIT = sf::seconds(5.0f);

// how many messages can be stored in input history.
// 60fps * send_rate to get number of inputs sent per tick, * 2 to account for slight variance
inline static constexpr size_t MAX_INPUT_HISTORY_SIZE = 60 * (INPUT_SEND_RATE.asSeconds()) * 2;

// how many messages can be stored in message history
// the larger this is, the more delay/drops can be handled without desyncs, but the higher the movement latency
inline static constexpr size_t MAX_MESSAGE_HISTORY_SIZE = 2;

// how long the host waits after starting a match before beginning the countdown
// the larger this is, the less likely other players are to start mid-countdown
inline static constexpr sf::Time HOST_MATCH_START_DELAY = sf::seconds(1);

// how long the host waits after ending a round before deciding the winner, to account for delays
inline static constexpr sf::Time HOST_DECIDE_WINNER_DELAY = sf::seconds(1);

// how long after round end before the host starts a new round
inline static constexpr sf::Time HOST_NEW_ROUND_DELAY = sf::seconds(3);