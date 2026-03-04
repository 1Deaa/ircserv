/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 19:27:37 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 19:27:38 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(fd_t fd) : Socket(fd, CLIENT)
{
	_state = RUNNING;
	registered_ = false;
	passed_ = false;
	nickSet_ = false;
	userSet_ = false;
}
Client::~Client() {}

void	Client::setIPAddress(const std::string &ip)
{
	_ipAddress = ip;
}

const std::string	&Client::getIPAddress(void) const
{
	return (_ipAddress);
}

std::string	&Client::getReadBuffer(void)
{
	return (_readBuffer);
}

std::string	&Client::getWriteBuffer(void)
{
	return (_writeBuffer);
}

void	Client::updatePollEvents(void)
{
	if (!_writeBuffer.empty())
		this->addEvent(POLLOUT);
	else
		this->removeEvent(POLLOUT);
}

void	Client::queueWrite(std::string str)
{
	str.append("\r\n");
	_writeBuffer.append(str);
}

void	Client::setState(CLIENT_STATE val)
{
	_state = val;
}

const CLIENT_STATE	&Client::getState(void) const
{
	return (_state);
}

const std::string	Client::getNick(void) const
{
	if (_nickName.empty())
		return ("*");
	return (_nickName);
}
