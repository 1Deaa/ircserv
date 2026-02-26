/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 19:34:40 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 19:34:40 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(fd_t fd, SocketType type)
{
	_prq.fd = fd;
	_prq.events = 0;
	_prq.revents = 0;
	_type = type;
}

Socket::~Socket()
{
	closeFd();
}

SocketType	Socket::getType(void) const {return (_type);}
fd_t	Socket::getFd(void) const {return (_prq.fd);}
short	Socket::getEvents(void) const {return (_prq.events);}
short	Socket::getRevents(void) const {return (_prq.revents);}
pollfd	&Socket::getPollFd(void) {return (_prq);}
void	Socket::setEvents(short events) {_prq.events = events;}
void	Socket::setRevents(short events) {_prq.revents = events;}
void	Socket::setFd(fd_t fd) {_prq.fd = fd;}
void	Socket::closeFd(void) {if (_prq.fd != -1) close(_prq.fd);}
void	Socket::addEvent(short event) {_prq.events |= event;}
void	Socket::removeEvent(short event) {_prq.events &= ~event;}
bool	Socket::hasRevent(short event) const {return ((_prq.revents & event) != 0);}

void	Socket::setNonBlocking(fd_t fd)
{
	if (-1 == fcntl(fd, F_SETFL, O_NONBLOCK))
		throw (std::runtime_error("failed to set O_NONBLOCK on socket."));
}

void	Socket::setNonBlocking(Socket *sock)
{
	setNonBlocking(sock->getFd());
}
