#pragma once
#include <SFML\Network.hpp>
#include "utils.h"
#include <deque>

class Player;		// Forward declaration

enum class ConnState {
	Disconnected,	// Player disconnected (will be removed from the game)
	NotReady,		// Not ready (default value, never assigned otherwise)
	Ready			// Player connected and ready
};

// Struct containing a packet, and the last status returned from send().
// Allows for easy retrying of partial sends and per-packet debugging if necessary
struct StatusPacket {
	sf::Packet packet;
	sf::Socket::Status status;

	StatusPacket(sf::Packet p) { 
		packet = p; 
		status = sf::Socket::Status::NotReady;
	}

	StatusPacket() {
		packet = sf::Packet();
		status = sf::Socket::Status::NotReady;
	}
};

class Connection
{
public:
	// construct a Connection from ip, port and id (tcpSocket contains port and id info)
	Connection(sf::IpAddress ip, unsigned short port, uint8_t uid);
	Connection(std::unique_ptr<sf::TcpSocket>, uint8_t uid);

	// the state of this connection 
	ConnState state = ConnState::NotReady;

	sf::TcpSocket* getSocket() const { return tcpSocket.get(); }
	sf::IpAddress getIp() const { return ipAddr; }
	unsigned short getRemotePort() const { return remotePort; }

	uint8_t getId() const { return id; }
	bool isHost() const { return id == 0; }

	void setName(std::string name) { playerName = name; }	// UNUSED
	std::string getName() const { return playerName; }		// UNUSED
	void setPlayer(Player* plr) { player = plr; }
	Player* getPlayer() const { return player; }

	// list of packets to be sent over TCP
	std::vector<StatusPacket> eventPacketsToSend;
	// packet to be sent over UDP
	StatusPacket updatePacketToSend;

	sf::Clock latencyClock;						// clock for tracking time between pings and ping chains
	std::deque<float> lastPingTimes;			// the last times between pings
	float roundTripLatency = -1.0f;				// latency between host and client, default value signals to start a new chain

private:
	// tcp socket for this connection
	// host has one for every connection, but players only have this for the host
	std::unique_ptr<sf::TcpSocket> tcpSocket;			

	sf::IpAddress ipAddr = sf::IpAddress::LocalHost;	// ip address of connection
	unsigned short remotePort;							// remote port of connection

	Player* player = nullptr;		// pointer to corresponding player object

	std::string playerName;			// the player's name (UNUSED)
	uint8_t id = 0;					// the connection ID (0 for host)
};

