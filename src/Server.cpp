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

Server::Server(int port, const std::string &password): _port(port), _password(password)
{
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
	for (std::size_t i = 0; i < _sockets.size(); i++)
		delete (_sockets[i]);
	_sockets.clear();
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
				std::cout << RED << "[-] Client <" << _disconnected[i]->getFd() << "> disconnected!" << RESET << std::endl;
				disconnectSocket(_disconnected[i]);
			}
			_disconnected.clear();
		}
	}
}

void	Server::executeCommand(Client *client, const std::string &cmd)
{
	if (cmd == "Ping")
	{
		client->queueWrite("Pong");
	}
	else
	{
		client->queueWrite("Unknown Command");
	}
}

void	Server::processBuffer(Client *client)
{
	std::size_t	pos;
	std::string	&buffer = client->getReadBuffer();
	if (buffer.size() > 8192)
	{
		markClosing(client);
		client->queueWrite("ERROR: Input buffer overflow");
		return ;
	}
	while ((pos = buffer.find(CRLF)) != std::string::npos)
	{
		std::string	cmd = buffer.substr(0, pos);
		buffer.erase(0, pos + 2);
		if (cmd.size() > 510)
		{
			markClosing(client);
			client->queueWrite("ERROR: Line too long");// <----
			return ;
		}
		executeCommand(client, cmd);
	}
}

void	Server::receiveData(Client *client)
{
	if (CLOSING == client->getState())
		return ;
	char	buff[4096];

	std::memset(buff, 0, sizeof(buff));
	ssize_t bytes = recv(client->getFd(), buff, sizeof(buff) - 1, 0);
	if (bytes <= 0)
	{
		markDisconnected(client);
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
		if (buffer.empty() && client->getState() == CLOSING)
			markDisconnected(client);
	}
	else
	{
		markDisconnected(client);
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
		client->setState(DISCONN);
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

	std::cout << GREEN <<"[+] Client <" << client->getFd() << "> IP: " << client->getIPAddress() << " connected!" << RESET << std::endl;
}

void	Server::markClosing(Client *client)
{
	client->setState(CLOSING);
	client->removeEvent(POLLIN);
}