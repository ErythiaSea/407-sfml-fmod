#pragma once
#include <SFML/Network.hpp>
#include <deque>
#include <bitset>
#include "utils.h"
#include "Connection.h"
#include "Player.h"
#include "Messages.h"
#include "EnemyManager.h"

// helper class for visitor (may have to move this later)
// https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

class NetworkManager
{
public:
    NetworkManager() {};

    // Configures sockets and ports.
    void setupNetworking();
    // Connect players to the host player.
    void connectToHost();
    // Bind UDP socket.
    void bindUdpSocket(unsigned short port);
    // Listen for incoming connections.
    void listen(std::vector<GameEvent>& eventsRecieved);

    // Create event packets for TCP.
    void createEventPackets(std::vector<GameEvent>& eventsToSend);
    // Send event packets.
    void sendEventPackets();
    // Receive incoming event packets.
    void receiveEventPackets(std::vector<GameEvent>& eventsToProcess);

    // Create input command packet for TCP.
    void createInputPacket(PlayerUpdateMessage updateMessage);
    // Handle received input command packets.
    void receiveInputPacket(sf::Packet& packet, Connection& con);

    // Create state sync packets for UDP.
    void createStatePackets(GhostsUpdateMessage ghostsUpdate);
    // Send a packet over UDP.
    void sendStatePackets();
    // Receive incoming state sync packets.
    void receiveStatePackets(EnemyManager& enemyManager);

    // Start new ping chains with clients.
    void startPingChain(float gameTime);
    // Respond to a received ping.
    float sendReplyPing(PingMessage receivedPing, Connection& sender);

    // Announce that a connection has changed to all players.
    void announceConnectionChange(Connection& con, bool join);
    // Handle the new dis/connection.
    void handleConnectionChange(sf::Packet& packet, std::vector<GameEvent>& eventsReceived);
    // Send the host's connection list to a player, and inform them of their ID.
    void sendConnectionList(Connection& receiver);
    // Update the connection list.
    void updateConnectionList(sf::Packet& packet, std::vector<GameEvent>& eventsRecieved);
        
    // Add a new connection, and a new player to the game.
    void addConnection(Connection con, std::vector<GameEvent>& eventsRecieved);
    // Deal with disconnections last.
    void handleDisconnections();

    static bool isHost() { return b_host; }
    unsigned short getPort() const { return (b_host ? hostPort : playerPort); }
    uint8_t getId() const { return myId; }

    // Returns the connection with an associated id.
    Connection* getConnectionWithId(uint8_t id);

    // For handling sending different packet types.
    sf::Clock eventSendTimer;
    sf::Clock inputSendTimer;
    bool eventSendReady() const { return eventSendTimer.getElapsedTime() > EVENT_SEND_RATE; }
    bool inputSendReady() const { return inputSendTimer.getElapsedTime() > INPUT_SEND_RATE; }

    // debug
    void printConnections();

private:
    inline static bool b_host = true;
    unsigned short hostPort = 54940; // :v:
    unsigned short playerPort = sf::Socket::AnyPort;

    sf::TcpListener listener;
    bool listening = false;
    sf::SocketSelector selector;
    sf::UdpSocket udpSocket;

    // we accept udp packets with this id or higher
    // this is also the variable the host uses to determine the id of sent packets
    uint16_t expectedUdpPacketId = 0;  

    std::vector<Connection> connectedPlayers;
    Connection* hostConnection = nullptr;

    bool idInUse[MAX_PLAYERS] = { false };
    int8_t firstFreeId() const;
    uint8_t myId;
};

