/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 19:27:26 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 19:27:26 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::printError(const std::string &msg)
{
	std::cerr << "ircserv: " << msg << std::endl;
}

void	Server::printClientLog(Client *client, LOG type)
{
	printClientLog(client, type, "");
}

void	Server::printClientLog(Client *client, LOG type, const std::string &msg)
{
	switch (type)
	{
		case CONNLOG:
			std::cout << GREEN << "[+] Client <" << client->getFd() << "> IP: " << client->getIPAddress() << " connected!" << RESET << std::endl;
			break ;
		case ERRLOG:
			std::cout << YELLOW << "[*] Client <" << client->getFd() << "> " << msg << RESET << std::endl;
			break ;
		case DISCLOG:
			std::cout << RED << "[-] Client <" << client->getFd() << "> disconnected!" << RESET << std::endl;
			break ;
		case NORMLOG:
			std::cout << BLUE << "[/] Client <" << client->getFd() << "> " << msg << RESET << std::endl;
		default:
			break;
	}
}

Server::Server(int port, const std::string &password): _port(port), _serverName("Discodo"), _password(password)
{
	_commandMap["PASS"] = &Server::handlePass;
	_commandMap["NICK"] = &Server::handleNick;
	_commandMap["USER"] = &Server::handleUser;
	_commandMap["QUIT"] = &Server::handleQuit;
	_commandMap["JOIN"] = &Server::handleJoin;
	_commandMap["PRIVMSG"] = &Server::handlePrivmsg;
	_commandMap["MODE"] = &Server::handleMode;
	_commandMap["PART"] = &Server::handlePart;
	_commandMap["KICK"] = &Server::handleKick;
	_commandMap["TOPIC"] = &Server::handleTopic;
	_commandMap["INVITE"] = &Server::handleInvite;
	_commandMap["WHO"] = &Server::handleWho;
}

bool	Server::_signal = false;
void	Server::signalHandler(int signo)
{
	(void)signo;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::_signal = true;
}

Server::~Server()
{
	for (std::size_t i = 0; i < _sockets.size(); ++i)
		delete (_sockets[i]);
	_sockets.clear();
	for (std::size_t i = 0; i < _channels.size(); ++i)
		delete (_channels[i]);
	_channels.clear();
}

void	Server::selfSocket(void)
{
	struct sockaddr_in	addr;
	int					optval = 1;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);
	
	_selfSocket = new Socket(-1, SERVER);				// <---|‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
	_selfSocket->setFd(socket(AF_INET, SOCK_STREAM, 0));//     | Create Server Socket |
														//     |______________________|
	if (-1 == _selfSocket->getFd())
		throw (std::runtime_error("failed to create socket."));
	if (-1 == setsockopt(_selfSocket->getFd(), SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
		throw (std::runtime_error("failed to set SO_REUSEADDR on socket."));
	Socket::setNonBlocking(_selfSocket);
	if (-1 == bind(_selfSocket->getFd(), (struct sockaddr *)&addr, sizeof(addr)))
		throw (std::runtime_error("failed to bind socket."));
	if (-1 == listen(_selfSocket->getFd(), SOMAXCONN))
		throw (std::runtime_error("failed to listen on socket."));
	
	_selfSocket->setEvents(POLLIN); //     |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
	_selfSocket->setRevents(0);     //     | Add Server Socket |
	_sockets.push_back(_selfSocket);// <---|___________________|
}

void	Server::run()
{
	selfSocket();

	std::cout << GREEN << "Server Listening Socket <" << _selfSocket->getFd() << "> Started" << RESET <<std::endl;
	std::cout << "Waiting for client connection...\n";

	while (!Server::_signal)
	{
		std::vector<pollfd>	pfds;
		pfds.reserve(_sockets.size());

		for (std::size_t i = 0; i < _sockets.size(); i++)
		{
			_sockets[i]->updatePollEvents();
			pfds.push_back(_sockets[i]->getPollFd());
		}

		if (0 >= poll(pfds.data(), pfds.size(), -1))
			continue ;
														 //     |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
		for (std::size_t i = 0; i < _sockets.size(); i++)//     | Propagate Revents |
			_sockets[i]->setRevents(pfds[i].revents);    // <---|___________________|
		
		for (std::size_t i = 0; i < _sockets.size(); i++)
		{
			Socket	*sock = _sockets[i];

			if (!sock->getRevents())
				continue ;
			if (sock->hasRevent(POLLIN))
			{							//      |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
				switch (sock->getType())// <--- | Switch Behaviour |
				{						//      |__________________|
					case (SERVER):
						acceptClient();
						break ;
					case (CLIENT):
						receiveData(static_cast<Client*>(sock));
						break ;
				}
			}
			if (sock->hasRevent(POLLOUT) && sock->getType() == CLIENT)
			{
				sendData(static_cast<Client*>(sock));
			}
		}							//      |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
		if (!_disconnected.empty()) // <--- | Disconnect Clients |
		{							//      |____________________|
			for (std::size_t i = 0; i < _disconnected.size(); i++)
			{
				printClientLog(static_cast<Client*>(_disconnected[i]), DISCLOG);
				disconnectSocket(_disconnected[i]);
			}
			_disconnected.clear();
		}
	}
}

void	Server::processBuffer(Client *client)
{
	std::size_t	pos;
	std::string	&buffer = client->getReadBuffer();
	if (buffer.size() > 4096)
	{
		markClosing(client);
		printClientLog(client, ERRLOG, "caused buffer overflow!");
		return ;
	}
	while ((pos = buffer.find(CRLF)) != std::string::npos)
	{
		std::string	cmd = buffer.substr(0, pos);
		buffer.erase(0, pos + 2);
		if (cmd.size() > 510)
		{
			markClosing(client);
			printClientLog(client, ERRLOG, "sent a long line exceeding 512 bytes");
			return ;
		}
		executeCommand(client, Command(cmd));
	}
}

void	Server::receiveData(Client *client)
{
	if (CLOSING == client->getNetworkState())
		return ;
	char	buff[4096];

	std::memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(client->getFd(), buff, sizeof(buff) - 1, 0);
	if (bytes <= 0)
	{
		handleQuit(client);
		// markDisconnected(client);
		return ;
	}
	std::string		&buffer = client->getReadBuffer();
	buffer.append(buff, bytes);
	processBuffer(client);
}

void	Server::sendData(Client *client)
{
	std::string	&buffer = client->getWriteBuffer();

	ssize_t	sent = send(client->getFd(), buffer.data(), buffer.size(), 0);

	if (sent > 0)
	{
		buffer.erase(0, sent);
		if (buffer.empty() && client->getNetworkState() == CLOSING)
			handleQuit(client);
			// markDisconnected(client);
	}
	else
	{
		handleQuit(client);
		// markDisconnected(client);
	}
}

void	Server::disconnectSocket(Socket *socket)
{
	_sockets.erase(std::remove(_sockets.begin(), _sockets.end(), socket), _sockets.end());
	delete (socket);
}

void	Server::markDisconnected(Client *client)
{
	std::vector<Socket*>::iterator	it = std::find(_disconnected.begin(), _disconnected.end(), client);
	if (it == _disconnected.end())
	{
		client->setNetworkState(DISCONN);
		_disconnected.push_back(client);
	}
}

void	Server::acceptClient(void)
{
	struct sockaddr_in	addr;
	socklen_t			len = sizeof(addr);

	int	clFd = accept(_selfSocket->getFd(), (sockaddr*)&addr, &len);
	if (-1 == clFd)
	{
		Server::printError("failed while invoking accept().");
		return ;
	}
	if (-1 == fcntl(clFd, F_SETFL, O_NONBLOCK))
	{
		Server::printError("failed while invoking fcntl().");
		close(clFd);
		return ;
	}

	Client	*client = new Client(clFd);
	client->setEvents(POLLIN);
	client->setRevents(0);
	client->setIPAddress(inet_ntoa(addr.sin_addr));
	_sockets.push_back(client);
	printClientLog(client, CONNLOG);
}

void	Server::markClosing(Client *client)
{
	client->setNetworkState(CLOSING);
	client->removeEvent(POLLIN);
	client->setRevents(0);
}

void	Server::tryRegister(Client *client)
{
	int	required = LOGIN_PASS | LOGIN_NICK | LOGIN_USER;

	if (client->hasLoginState(LOGIN_REGS))
		return ;
	if ((client->getLoginState() & required) == required) 
	{
		if (client->getPassword() != _password)
		{
			client->queueWrite(ERR_PASSWDMISMATCH(_serverName, client->getNick()));
			printClientLog(client, ERRLOG, "entered incorrect password.");
			markClosing(client);
			return ;
		}
		printClientLog(client, NORMLOG, "entered correct password.");
		client->addLoginState(LOGIN_REGS);
		client->queueWrite(RPL_WELCOME(_serverName, client->getNick(), client->getUser(), client->getIPAddress()));
		printClientLog(client, NORMLOG, "registered successfully!");
	}
}

bool	Server::nickExists(Client *client, const std::string &nick)
{
	Client	*otherClient;
	for (std::size_t i = 0; i < _sockets.size(); i++)
	{
		if (_sockets[i]->getType() != CLIENT)
			continue ;
		otherClient = static_cast<Client*>(_sockets[i]);
		if (Command::toUpper(otherClient->getNick()) == Command::toUpper(nick) && otherClient != client)
			return (true);
	}
	return (false);
}

Channel	*Server::getChannel(const std::string &_name)
{
	std::string	name = Command::toUpper(_name);
	for (std::size_t i = 0; i < _channels.size(); ++i)
	{
		if (_channels[i]->getName() == name)
			return (_channels[i]);
	}
	return (NULL);
}

Channel	*Server::createChannel(const std::string &_name)
{
	std::string name = Command::toUpper(_name);
	Channel	*channel = new Channel(name);
	_channels.push_back(channel);
	return (channel);
}

void	Server::broadcast(Channel *channel, const std::string &msg)
{
	broadcast(channel, msg, NULL);
}

void	Server::broadcast(Client *client, const std::string &msg)
{
	std::vector<Client*> notified;
	const std::vector<Channel*> &channels = client->getChannels();

	for (std::size_t i = 0; i < channels.size(); ++i)
	{
		const std::vector<Client*> &members = channels[i]->getMembers();

		for (std::size_t j = 0; j < members.size(); ++j)
		{
			Client *target = members[j];

			if (target == client)
				continue;

			bool already = false;

			for (std::size_t k = 0; k < notified.size(); ++k)
			{
				if (notified[k] == target)
				{
					already = true;
					break;
				}
			}

			if (!already)
			{
				target->queueWrite(msg);
				notified.push_back(target);
			}
		}
	}
}

void	Server::broadcast(Channel *channel, const std::string &msg, Client *exclude)
{
	if (!channel)
		return ;
	const std::vector<Client*>	&members = channel->getMembers();
	for (size_t i = 0; i < members.size(); ++i)
	{
		if (members[i] != exclude)
			members[i]->queueWrite(msg);
	}
}

Client	*Server::getClient(const std::string &_str)
{
	Client	*client;
	const std::string str = Command::toUpper(_str);

	for (std::size_t i = 0; i < _sockets.size(); i++)
	{
		if (_sockets[i]->getType() != CLIENT)
			continue ;
		client = static_cast<Client*>(_sockets[i]);
		if (Command::toUpper(client->getNick()) == str && client->hasLoginState(LOGIN_REGS))
			return (client);
	}
	return (NULL);
}

void	Server::removeChannel(Channel *channel)
{
	for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (*it == channel)
		{
			_channels.erase(it);
			delete (*it);
			return;
		}
	}
}