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

Server::Server(int port, std::string password): _port(port), _password(password)
{

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

	std::cout << "Server Listening <" << _selfSocket->getFd() << "> Started" << std::endl;
	std::cout << "Waiting to accept client connection...\n";

	while (!Server::_signal)
	{
		std::vector<pollfd>	pfds;
		pfds.reserve(_sockets.size());

		for (std::size_t i = 0; i < _sockets.size(); i++)
			pfds.push_back(_sockets[i]->getPollFd());

		if (0 >= poll(pfds.data(), pfds.size(), -1));
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
			{
				switch (sock->getType())
				{
					case (SERVER):
						acceptClient();
						break ;
					case (CLIENT):
						receiveData(dynamic_cast<Client*>(sock));
						break ;
				}
			}
		}
	}
}

void	Server::acceptClient(void)
{
	struct sockaddr_in	addr;
	socklen_t			len = sizeof(addr);

	int	clFd = accept(_selfSocket->getFd(), (sockaddr*)&addr, &len);
	if (-1 == clFd)
	{
		std::cerr << "failed while invoking accept()." << std::endl;
		return ;
	}
	if (-1 == fcntl(clFd, F_SETFL, O_NONBLOCK))
	{
		std::cerr << "failed while invoking fcntl()." << std::endl;
		close(clFd);
		return ; 
	}

	Client	*client = new Client(clFd);
	client->setEvents(POLLIN);
	client->setRevents(0);
	client->setIPAddress(inet_ntoa(addr.sin_addr));
	_sockets.push_back(client);

	std::cout << "Client <" << client->getFd() << "> connected!" << std::endl;
}