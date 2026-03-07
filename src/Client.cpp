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
	_networkState = RUNNING;
	_loginState = 0;
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
	if (str.length() > 510)
		str = str.substr(0, 510);
	str.append("\r\n");
	_writeBuffer.append(str);
}

void	Client::setNetworkState(CLIENT_NETWORK_STATE val)
{
	_networkState = val;
}

const CLIENT_NETWORK_STATE	&Client::getNetworkState(void) const
{
	return (_networkState);
}

void	Client::setNick(const std::string &str)
{
	_nickName = str;
}

void	Client::setPassword(const std::string &str)
{
	_cpassword = str;
}

void	Client::setUser(const std::string &str)
{
	_userName = str;
}

void	Client::setRealName(const std::string &str)
{
	_realName = str;
}

const std::string	Client::getUser(void) const
{
	return (_userName);
}

const std::string	Client::getRealName(void) const
{
	return (_realName);
}

const std::string	Client::getPassword(void) const
{
	return (_cpassword);
}

const std::string	Client::getPrefix(void) const
{
	return (getNick() + "!" + getUser() + "@" + getIPAddress());
}

const std::string	Client::getNick(void) const
{
	if (_nickName.empty())
		return ("*");
	return (_nickName);
}

void	Client::addLoginState(int val)
{
	_loginState |= val;
}

void	Client::rmvLoginState(int val)
{
	_loginState &= ~val;
}

int		Client::getLoginState(void) const
{
	return (_loginState);
}

bool	Client::hasLoginState(int val) const
{
	return ((_loginState & val) != 0);
}

const std::vector<Channel*>	&Client::getChannels(void) const
{
	return (_channels);
}

void	Client::addChannel(Channel *channel)
{
	_channels.push_back(channel);
}

void Client::rmvChannel(Channel *channel)
{
	for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (*it == channel)
		{
			_channels.erase(it);
			return;
		}
	}
}