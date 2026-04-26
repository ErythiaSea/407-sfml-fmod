#include "Connection.h"

Connection::Connection(sf::IpAddress ip, unsigned short port, uint8_t uid)
{
	ipAddr = ip;
	remotePort = port;

	playerName = std::to_string(remotePort);
	id = uid;

	Utils::printMsg("New connection w port: " + std::to_string(remotePort) + " and id:" + std::to_string(id), debug);
}

Connection::Connection(std::unique_ptr<sf::TcpSocket> socket, uint8_t uid)
{
	tcpSocket = std::move(socket);

	ipAddr = tcpSocket->getRemoteAddress().value_or(sf::IpAddress::LocalHost);
	remotePort = tcpSocket->getRemotePort();

	playerName = std::to_string(remotePort);
	id = uid;

	Utils::printMsg("New connection w port: " + std::to_string(remotePort) + " and id:" + std::to_string(id), debug);
}