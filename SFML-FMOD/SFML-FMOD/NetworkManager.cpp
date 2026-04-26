#include "NetworkManager.h"

void NetworkManager::setupNetworking()
{
    // Reserve space for maximum player count
    connectedPlayers.reserve(MAX_PLAYERS);
    
    // id 0 must be in use because we are either the host or connecting to them
    idInUse[0] = true;

    // Attempt twice to set up listener on default host port. If it fails, give option to host on other port, or to try joining as peer
    listener.setBlocking(false);
    udpSocket.setBlocking(false);
    sf::Socket::Status code = sf::Socket::Status::Error;
    auto port = (b_host ? hostPort : playerPort);

    while (code != sf::Socket::Status::Done) {
        port = (b_host ? hostPort : playerPort);
        for (int i = 0; i < 2; i++) {
            code = listener.listen(port, sf::IpAddress::LocalHost);
            if (b_host) {
                if (code == sf::Socket::Status::Error) {
                    Utils::printMsg("Failed to establish listener on host port " + std::to_string(port) + (i == 0 ? ", retrying..." : "."), warning);
                    listening = false;
                }
                else {
                    Utils::printMsg("Set up listener on host port " + std::to_string(listener.getLocalPort()) + "! Others can join through this port.\n", success);
                    b_host = true;
                    listening = true;
                    break;
                }
            }

            // does a player need a listener? maybe later, if the host DCs, so this player can become the host
            // but right now no. todo i guess
            else {
                Utils::printMsg("Not the host, so skipping listener bind...", warning);
                //if (code == sf::Socket::Status::Error) {
                //    Utils::printMsg("Failed to establish listener on port " + std::to_string(port) + (i == 1 ? ", retrying..." : "."), warning);
                //    listening = false;
                //}
                //else {
                //    Utils::printMsg("Set up listener on port " + std::to_string(listener.getLocalPort()) + "!\n", success);
                //    b_host = false;
                //    listening = true;
                //    break;
                //}
            }
        }
        if (code == sf::Socket::Status::Error) {
            bool validInput = false;
            while (!validInput) {
                Utils::printMsg("Press enter to join as a player.\nType a 5 digit number lower than 65535 to attempt to host on another port:", debug);
                std::string in = Utils::getUserInput();
                if (in.empty()) {
                    b_host = !b_host;
                    validInput = true;
                }
                else if (in.length() == 5) {
                    try {
                        unsigned short attemptPort = static_cast<unsigned short>(std::stoi(in));
                        validInput = true;
                        hostPort = attemptPort;
                    }
                    catch (...) {
                        Utils::printMsg("Invalid port!", error);
                    }
                }
                else Utils::printMsg("Bad input!", error);
            }
        }
    }

    // by now, we know if we're host or player, our port number, with a udpSocket bound to that port.
    // if we're the host, we're done, all there is to do now is wait for incoming connections
    if (b_host) {
        myId = 0;
        bindUdpSocket(listener.getLocalPort());
        return;
    }

    // but for players, this is not where the story ends...
    connectToHost();
}

void NetworkManager::connectToHost()
{
    std::unique_ptr<sf::TcpSocket> hostSocket = std::make_unique<sf::TcpSocket>();
    bool isConnected = false;
    bool firstAttempt = true;
    while (!isConnected) {

        // let users change the host port if they know it's changed
        bool validInput = false;
        while (!validInput) {
            std::string start = (firstAttempt ? "Press enter to use the default host port (54940).\n" : ("Press enter to keep using the same port (" + std::to_string(hostPort)));
            if (!firstAttempt) start.append(").\n");
            Utils::printMsg(start + "Type any 5 digit number lower than 65535 to change the host port (only do this if you know it's changed!)", debug);
            std::string in = Utils::getUserInput();
            if (in.empty()) {
                validInput = true;
            }
            else if (in.length() == 5) {
                try {
                    unsigned short attemptPort = static_cast<unsigned short>(std::stoi(in));
                    validInput = true;
                    hostPort = attemptPort;
                }
                catch (...) {
                    Utils::printMsg("Invalid port!", error);
                }
            }
            else Utils::printMsg("Bad input!", error);
        }

        firstAttempt = false;
        Utils::printMsg("Attempting connection to host...");

        // prefer to attempt connection in blocking mode, to avoid sfml "fiddliness"
        hostSocket->setBlocking(true);
        sf::Socket::Status status = hostSocket->connect(sf::IpAddress::LocalHost, hostPort, sf::seconds(10));
        if (status == sf::Socket::Status::Done) {
            Utils::printMsg("Connected!", MessageType::success);
            isConnected = true;
            hostSocket->setBlocking(false);
            connectedPlayers.push_back({ std::move(hostSocket), 0 }); //host id is always 0
            hostConnection = &connectedPlayers.back();
            selector.add(*hostConnection->getSocket());
        }
        else {
            Utils::printMsg("Failed to connect to host.", MessageType::error);
        }
    }

    bindUdpSocket(hostConnection->getSocket()->getLocalPort());
}

void NetworkManager::bindUdpSocket(unsigned short port)
{
    sf::Socket::Status code = sf::Socket::Status::Error;
    int attempt = 0;
    Utils::printMsg("Attempting to bind a UDP socket at port " + std::to_string(port) + "...");
    while (code != sf::Socket::Status::Done) {
        code = udpSocket.bind(port, sf::IpAddress::LocalHost);
        if (code == sf::Socket::Status::Done) {
            Utils::printMsg("UDP socket bound successfully at port " + std::to_string(udpSocket.getLocalPort()) + "!", success);
        }
        else {
            std::string msg = "Socket binding failed, retrying...";
            if (attempt > 5) msg.append(" (If you're stuck here, you should consider restarting.)");
            Utils::printMsg(msg);
        }
        attempt++;
    }
}

void NetworkManager::listen(std::vector<GameEvent>& eventsRecieved)
{
    if (!isHost()) return;

    // close listener if all player slots are full
    if (firstFreeId() == -1 && listening) {
        listener.close();
        listening = false;
        Utils::printMsg("Closed listener!");
        return;
    }

    // open listener if there's a free player slot
    if (firstFreeId() != -1 && !listening) {
        if (listener.listen(hostPort, sf::IpAddress::LocalHost) == sf::Socket::Status::Done) {
            listening = true;
            Utils::printMsg("Reopened listener!", success);
        }
        else {
            Utils::printMsg("Tried to reopen listener, but failed!", error);
        }
    }
    if (!listening) return;

    std::unique_ptr<sf::TcpSocket> socket = std::make_unique<sf::TcpSocket>();

    // Check for incoming connections
    if (listener.accept(*socket) == sf::Socket::Status::Done) {
        Utils::printMsg("New connection from: "
            + socket->getRemoteAddress().value().toString()
            + ":"
            + std::to_string(socket->getRemotePort()));

        // Add new player connection.
        uint8_t idx = firstFreeId();
        idInUse[idx] = true;
        addConnection({ std::move(socket), idx }, eventsRecieved);
    }
}

void NetworkManager::createEventPackets(std::vector<GameEvent>& eventList)
{
    // if there's no new events, no need to send anything!
    if (eventList.empty()) return;
    Utils::printMsg("Event list isn't empty!");

    // need to sort event list before processing, ascending order
    // if we have multiple of the same type we want to handle this concurrently
    std::sort(eventList.begin(), eventList.end(), [](GameEvent a, GameEvent b) { return a.type < b.type; });

    sf::Packet eventPacket;
    uint16_t prefix = 0;
    std::map<EventTypes, uint8_t> eventCounts;

    for (auto& ev : eventList) {
        prefix |= ev.type;
        eventCounts[ev.type]++;
    }

    // add event bitflag and event counts into packet
    eventPacket << prefix;
    for (auto it = eventCounts.begin(); it != eventCounts.end(); it++) {
        uint8_t c = it->second;
        eventPacket << c;
    }

    // add event data to packet based on type
    for (auto& ev : eventList) {
        std::visit(overloads{
            [&eventPacket](MatchUpdateMessage mum) { eventPacket << mum; },
            [&eventPacket](EnemySpawnMessage esm) { eventPacket << esm; },
            [&eventPacket](EnemyHurtMessage ehm) { eventPacket << ehm; },
            [&eventPacket](CoinsSpawnMessage csm) { eventPacket << csm; },
            [&eventPacket](CoinsSpawnRequestMessage csrm) { eventPacket << csrm; },
            [&eventPacket](CoinPickupMessage cpm) { eventPacket << cpm; },
            [&eventPacket](PlayerUpdateMessage pum) {eventPacket << pum;}, 

            [&eventPacket](ConnectionChangeMessage ccm) { /*handled in AnnounceConnectionChange()*/ },
            [&eventPacket](ConnectionListMessage clm) { /*handled in SendConnectionList()*/ },
            [&eventPacket](PingMessage pm) { /*handled in sendReplyPing()*/ },
            }, ev.var);
    }
    eventList.clear();

    // add this packet to the send list of everyone who should recieve it
    if (isHost()) {
        for (auto& p : connectedPlayers) {
            p.eventPacketsToSend.push_back(eventPacket);
        }
    }
    else hostConnection->eventPacketsToSend.push_back(eventPacket);
}

void NetworkManager::sendEventPackets()
{
    // send every packet to every player that needs it,
    // and erase successfully sent packets from the vector
    for (auto& plr : connectedPlayers) {
        
        // skip players without packets to send
        if (!plr.eventPacketsToSend.empty()) {

            for (auto& sp : plr.eventPacketsToSend) {
                sp.status = plr.getSocket()->send(sp.packet);
            }
            // clear packets we sent successfully
            std::erase_if(plr.eventPacketsToSend, [](StatusPacket sp) {return sp.status == sf::Socket::Status::Done; });
        }

        // early return so that clients only send to the first player (host)
        if (!isHost()) return;
    }
}

void NetworkManager::createInputPacket(PlayerUpdateMessage updateMessage)
{
    sf::Packet packet;
    packet << uint16_t(MovementInputs);
    packet << updateMessage;

    for (auto& con : connectedPlayers) {
        con.eventPacketsToSend.push_back(packet);
    }
}

void NetworkManager::receiveInputPacket(sf::Packet& packet, Connection& con)
{
    PlayerUpdateMessage msg;
    packet >> msg;

    // add the update message to the connection's associated player
    Connection* connection = getConnectionWithId(msg.id);
    if (connection == nullptr) return;
    //Utils::printMsg("Input Connection success!", debug);
    Player* player = connection->getPlayer();
    if (player == nullptr) return;
    //Utils::printMsg("Input Player success!", debug);
    player->addUpdateMessage(msg);
}

void NetworkManager::createStatePackets(GhostsUpdateMessage ghostsUpdate)
{
    // we ain't update no ghosts! (to the tune of Ghostbusters)
    if (ghostsUpdate.num_ghosts == 0) return;

    // set update packet id and increment for the next one
    ghostsUpdate.packet_id = expectedUdpPacketId;
    expectedUdpPacketId++;

    // fill out packet with update information
    sf::Packet packet;
    packet << ghostsUpdate;

    // prepare to send it to all players
    for (auto& con : connectedPlayers) {
        con.updatePacketToSend = packet;
    }
}

void NetworkManager::sendStatePackets()
{
    for (auto& con : connectedPlayers) {
        // skip sending a packet that's been sent successfully
        if (con.updatePacketToSend.status == sf::Socket::Status::Done) continue;

        con.updatePacketToSend.status = udpSocket.send(con.updatePacketToSend.packet, con.getIp(), con.getRemotePort());
    }
}

void NetworkManager::receiveEventPackets(std::vector<GameEvent>& eventsToProcess)
{
    // no events to receive
    if (!selector.wait(sf::microseconds(50.f))) return;

    for (auto& plr : connectedPlayers) {
        if (!selector.isReady(*plr.getSocket())) continue;
        sf::Packet packet;

        // todo: error handle better
        auto code = sf::Socket::Status::Done;
        while (code == sf::Socket::Status::Done) {
            code = plr.getSocket()->receive(packet);
            if (code == sf::Socket::Status::Disconnected) {
                plr.state = ConnState::Disconnected;
                Utils::printMsg(std::to_string(plr.getId()) + " disconnected!");
                ConnectionChangeMessage ccm;
                ccm.join = false;
                ccm.con.id = plr.getId();
                eventsToProcess.push_back({ ConnectionChange, ccm });
                continue;
            }
            else if (code != sf::Socket::Status::Done) continue;

            // event bitflag
            uint16_t pfx;
            packet >> pfx;

            // event type to number of events
            std::map<uint16_t, uint8_t> typeCounts;

            // handle movement inputs separately
            if (pfx == MovementInputs) {
                receiveInputPacket(packet, plr);
            }

            // packets that aren't movement have more data to read
            else {
                for (size_t i = 0; i < 16; i++) {
                    if (!(pfx & (1 << i))) continue;
                    uint8_t c;
                    packet >> c;
                    typeCounts[1 << i] = c;
                }
            }

            // process events
            if (pfx & ConnectionChange) {
                for (int i = 0; i < typeCounts[ConnectionChange]; i++) {
                    Utils::printMsg("A player (dis)connected");
                    handleConnectionChange(packet, eventsToProcess);
                }
            }
            if (pfx & ConnectionList) {
                for (int i = 0; i < typeCounts[ConnectionList]; i++) {
                    Utils::printMsg("Updating connection list");
                    updateConnectionList(packet, eventsToProcess);
                }
            }
            if (pfx & MatchUpdate) {
                for (int i = 0; i < typeCounts[MatchUpdate]; i++) {
                    Utils::printMsg("Received match update");
                    MatchUpdateMessage mum;
                    packet >> mum;
                    eventsToProcess.push_back({ MatchUpdate, mum });
                    
                    // reset udp packet id at the start of a new round to avoid dealing with uint16_t cap/overflow
                    if (mum.round_start) {
                        expectedUdpPacketId = 0;
                    }
                }
            }
            if (pfx & EnemySpawn) {
                for (int i = 0; i < typeCounts[EnemySpawn]; i++) {
                    Utils::printMsg("An enemy spawned");
                    EnemySpawnMessage esm;
                    packet >> esm;
                    eventsToProcess.push_back({ EnemySpawn, esm });
                }
            }
            if (pfx & EnemyHurt) {
                for (int i = 0; i < typeCounts[EnemyHurt]; i++) {
                    Utils::printMsg("An enemy took damage");
                    EnemyHurtMessage ehm;
                    packet >> ehm;
                    eventsToProcess.push_back({ EnemyHurt, ehm });
                }
            }
            if (pfx & CoinsSpawnRequest) {
                for (int i = 0; i < typeCounts[CoinsSpawnRequest]; i++) {
                    Utils::printMsg("Received request for coin spawn");
                    CoinsSpawnRequestMessage csrm;
                    packet >> csrm;
                    eventsToProcess.push_back({ CoinsSpawnRequest, csrm });
                }
            }
            if (pfx & CoinsSpawn) {
                for (int i = 0; i < typeCounts[CoinsSpawn]; i++) {
                    Utils::printMsg("Some coins spawned");
                    CoinsSpawnMessage csm;
                    packet >> csm;
                    eventsToProcess.push_back({ CoinsSpawn, csm });
                }
            }
            if (pfx & CoinPickup) {
                for (int i = 0; i < typeCounts[CoinPickup]; i++) {
                    Utils::printMsg("Coin despawned");
                    CoinPickupMessage cpm;
                    packet >> cpm;
                    eventsToProcess.push_back({ CoinPickup, cpm });
                }
            }
            if (pfx & Ping) {
                // no for loop necessary, there can only ever be 1
                PingMessage ping;
                packet >> ping;

                // discard ping not meant for this client (forwarded from host)
                if (ping.receiver_id == myId || isHost()) {
                    Utils::printMsg("Got a ping! (Num: " + std::to_string(ping.ping_num) + ", ID: " + std::to_string(plr.getId()));
                    float t = sendReplyPing(ping, plr);
                    if (t >= 0.0f) {
                        ping.game_time = t;
                        eventsToProcess.push_back({ Ping, ping });
                    }
                }
            }

            // add this packet to other connections for forwarding, if it isn't just a ping (other clients will discard it anyway)
            if (isHost() && pfx != Ping) {
                for (auto& fwdPlr : connectedPlayers) {
                    if (&fwdPlr == &plr) continue;
                    fwdPlr.eventPacketsToSend.push_back(packet);
                }
            }
        }
        // this stops clients from checking non-existent tcp sockets of other players (host only).
        // inelegant? somewhat. functional? you bet!
        if (!isHost()) break;
    }
}

void NetworkManager::receiveStatePackets(EnemyManager& enemyManager)
{
    std::optional<sf::IpAddress> senderIp = sf::IpAddress::LocalHost;
    unsigned short senderPort;
    sf::Packet statePacket;

    while (udpSocket.receive(statePacket, senderIp, senderPort) == sf::Socket::Status::Done) {
        GhostsUpdateMessage gum;
        statePacket >> gum;
        
        // discard old packet if we've received something newer
        if (gum.packet_id < expectedUdpPacketId) {
            // account for possible uint16_t overflow
            if ((expectedUdpPacketId - gum.packet_id) < 60000) continue;
        }

        // this is a good packet, so do network updates with it, and only expect packets with a higher id
        enemyManager.networkUpdate(gum);
        expectedUdpPacketId = gum.packet_id + 1;
    }
}

void NetworkManager::startPingChain(float gameTime)
{
    // only the host does this
    if (!isHost()) return;

    MessagePrefix pfx;
    pfx.events = EventTypes::Ping;

    // loop through all players, every PING_WAIT seconds,
    // clear their ping times and start a new chain
    for (auto& con : connectedPlayers) {
        if (con.latencyClock.getElapsedTime() <= PING_WAIT && con.roundTripLatency != -1.0f) continue;
        con.lastPingTimes.clear();

        if (con.roundTripLatency == -1.0f) con.roundTripLatency = 0.0f;

        PingMessage ping;
        ping.ping_num = 0;
        ping.latency = 0.0f;
        ping.receiver_id = con.getId();
        ping.game_time = gameTime;

        sf::Packet packet;
        packet << pfx.events;
        packet << uint8_t(1);
        packet << ping;
        con.eventPacketsToSend.push_back(packet);
        con.latencyClock.restart();
        Utils::printMsg("Starting ping chain with id " + std::to_string(con.getId()), warning);
    }
}

float NetworkManager::sendReplyPing(PingMessage receivedPing, Connection& sender)
{
    MessagePrefix pfx;
    pfx.events = EventTypes::Ping;

    // host gets delay, and sends round trip latency to client after 4 pings
    if (isHost()) {
        float delay = sender.latencyClock.restart().asSeconds();
        sender.lastPingTimes.push_back(delay);
        receivedPing.ping_num++;

        // calculate latency after 4 pings
        if (receivedPing.ping_num == 4) {
            float delaySum = 0.0f;
            for (float t : sender.lastPingTimes) {
                delaySum += t;
            }
            sender.roundTripLatency = (delaySum / 4.0f);
            receivedPing.latency = sender.roundTripLatency;
            receivedPing.game_time += delaySum;
            Utils::printMsg("ID " + std::to_string(sender.getId()) + " has latency " + std::to_string(receivedPing.latency), debug);
        }
    }
    // clients set their latency from packet data
    else {
        if (receivedPing.ping_num == 4) {
            sender.roundTripLatency = receivedPing.latency;
            Utils::printMsg("ID " + std::to_string(sender.getId()) + " has latency " + std::to_string(receivedPing.latency), debug);

            // return estimated time value so we can update Game::gameTime
            return (receivedPing.game_time + (sender.roundTripLatency/2));
        }
    }

    sf::Packet packet;
    packet << pfx.events;
    packet << uint8_t(1);
    packet << receivedPing;
    sender.eventPacketsToSend.push_back(packet);

    // default return value signals not to update game time
    return -1.0f;
}

void NetworkManager::announceConnectionChange(Connection& con, bool join)
{
    MessagePrefix pfx;

    pfx.events = EventTypes::ConnectionChange;
    sf::Packet packet;

    packet << pfx.events << uint8_t(1) << con.getIp().toInteger() << static_cast<uint16_t>(con.getRemotePort()) << con.getId() << con.getName() << join;

    for (auto it = connectedPlayers.begin(); it < connectedPlayers.end(); it++) {
        Connection* plr = &*it;
        if (plr->getId() == con.getId()) continue;

        Utils::printMsg("Sending connection change to id " + std::to_string(plr->getId()), debug);
        plr->eventPacketsToSend.push_back(packet);
    }
}

void NetworkManager::handleConnectionChange(sf::Packet& packet, std::vector<GameEvent>& eventsRecieved) {
    ConnectionChangeMessage msg;
    packet >> msg.con.ip >> msg.con.port >> msg.con.id >> msg.con.name >> msg.join;

    // add new player
    if (msg.join) {
        addConnection({ sf::IpAddress(msg.con.ip), msg.con.port, msg.con.id }, eventsRecieved);
    }
    // remove player with matching id
    else {
        eventsRecieved.push_back({ ConnectionChange, msg });
        for (auto it = connectedPlayers.begin(); it < connectedPlayers.end(); it++) {
            Connection* plr = &*it;
            if (plr->getId() == msg.con.id) {
                plr->state = ConnState::Disconnected;
                break;
            }
        }
    }
}

void NetworkManager::sendConnectionList(Connection& receiver)
{
    MessagePrefix pfx;
    ConnectionListMessage msg;

    pfx.events = EventTypes::ConnectionList;
    msg.num = connectedPlayers.size() - 1; // -1 bc host wont resend its own connection and player knows its own connection

    msg.receiver_id = receiver.getId();
    sf::Packet packet;

    int i = 0;
    for (Connection& con : connectedPlayers) {
        if (con.getId() == receiver.getId()) continue;

        msg.con[i].ip = con.getIp().toInteger();
        msg.con[i].port = static_cast<uint16_t>(con.getRemotePort());
        msg.con[i].id = con.getId();
        msg.con[i].name = con.getName();
        i++;
    }

    // pack data
    packet << pfx.events << uint8_t(1) << msg.num << msg.receiver_id;
    for (i = 0; i < msg.num; i++) {
        packet << msg.con[i].ip << msg.con[i].port << msg.con[i].id << msg.con[i].name;
    }

    Utils::printMsg("Sending a connection list to id: " + std::to_string(receiver.getId()));
    receiver.eventPacketsToSend.push_back(packet);
}

void NetworkManager::updateConnectionList(sf::Packet& packet, std::vector<GameEvent>& eventsRecieved)
{
    ConnectionListMessage msg;
    packet >> msg.num >> msg.receiver_id;

    myId = msg.receiver_id;
    Utils::printMsg("My ID is " + std::to_string(myId));
    eventsRecieved.push_back({ ConnectionList, msg });

    if (msg.num == 0) return;

    // clear vector after first connection (host)
    while (connectedPlayers.begin() + 1 != connectedPlayers.end()) {
        connectedPlayers.erase(connectedPlayers.begin() + 1);
    }

    for (int i = 0; i < msg.num; i++) {
        packet >> msg.con[i].ip >> msg.con[i].port >> msg.con[i].id >> msg.con[i].name;
        addConnection({ sf::IpAddress(msg.con[i].ip), msg.con[i].port, msg.con[i].id }, eventsRecieved);
    }
}

void NetworkManager::addConnection(Connection con, std::vector<GameEvent>& eventsRecieved)
{
    Utils::printMsg("Hello from addConnection!", debug);
    connectedPlayers.push_back(std::move(con));
    if (connectedPlayers.back().getSocket() != nullptr) { // a tcp socket exists;
        selector.add(*connectedPlayers.back().getSocket()); // Add to persistent selector
        connectedPlayers.back().getSocket()->setBlocking(false);
    }
    if (isHost()) {
        announceConnectionChange(connectedPlayers.back(), true);
        sendConnectionList(connectedPlayers.back());
    }

    ConnectionChangeMessage ccm;
    PlayerConnection plrcon;
    plrcon.id = connectedPlayers.back().getId();
    ccm.con = plrcon;
    ccm.join = true;
    eventsRecieved.push_back({ ConnectionChange, ccm });
}

void NetworkManager::handleDisconnections()
{
    for (auto& con : connectedPlayers) {
        if (con.state == ConnState::Disconnected) {
            if (isHost()) {
                announceConnectionChange(con, false);
            }
            idInUse[con.getId()] = false;
        }
    }
    auto num_erased = std::erase_if(connectedPlayers, [](Connection& c) {return c.state == ConnState::Disconnected;});
    if (num_erased == 0) return;
    Utils::printMsg("Disconnected a player!");

    // repopulate selector if host
    // we need to do this since the pointers to sockets may be misaligned due to contents of vector moving
    if (!isHost()) return;

    selector.clear();
    for (auto& con : connectedPlayers) {
        selector.add(*(con.getSocket()));
        con.getPlayer()->setConnection(&con);
        //Utils::printMsg("argh");
    }
}

Connection* NetworkManager::getConnectionWithId(uint8_t id)
{
    for (auto& con : connectedPlayers) {
        if (con.getId() == id) return &con;
    }
    return nullptr;
}

void NetworkManager::printConnections()
{
    for (auto& p : connectedPlayers) {
        Utils::printMsg("IP: " + p.getIp().toString() + ", port: " + std::to_string(p.getRemotePort()));
    }
}

int8_t NetworkManager::firstFreeId() const
{
    for (int8_t i = 0; i < MAX_PLAYERS; i++) {
        if (!idInUse[i]) return i;
    }
    return -1;
}
