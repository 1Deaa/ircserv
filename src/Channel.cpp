/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 12:40:12 by drahwanj          #+#    #+#             */
/*   Updated: 2026/03/07 12:40:13 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

const std::string	&Channel::getName(void) const
{
	return (_name);
}

const std::string	&Channel::getTopic(void) const
{
	return (_topic);
}

const std::vector<Client*>	&Channel::getMembers(void) const
{
	return (_members);
}

const std::vector<Client*>	&Channel::getOperators(void) const
{
	return (_operators);
}

void	Channel::setName(const std::string &str)
{
	_name = str;
}

void	Channel::setTopic(const std::string &str)
{
	_topic = str;
}

void	Channel::addMember(Client *client)
{
	_members.push_back(client);
}

void	Channel::addOperator(Client *client)
{
	_operators.push_back(client);
}

bool	Channel::isMember(Client *client) const
{
	for (std::size_t i = 0; i < _members.size(); ++i)
	{
		if (_members[i] == client)
			return (true);
	}
	return (false);
}

bool	Channel::isOperator(Client *client) const
{
	for (std::size_t i = 0; i < _operators.size(); ++i)
	{
		if (_operators[i] == client)
			return (true);
	}
	return (false);
}

void	Channel::removeMember(Client *client)
{
	removeOperator(client);
	for (std::vector<Client*>::iterator it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it == client)
		{
			_members.erase(it);
			return ;
		}
	}
}

void	Channel::removeOperator(Client *client)
{
	for (std::vector<Client*>::iterator it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (*it == client)
		{
			_operators.erase(it);
			return ;
		}
	}
}

Channel::Channel(const std::string &str): _name(str)
{
}

Channel::~Channel()
{

}