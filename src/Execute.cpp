/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Execute.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:46:28 by drahwanj          #+#    #+#             */
/*   Updated: 2026/03/04 19:46:29 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	Server::executeCommand(Client *client, const Command &cmd)
{
	if (cmd.getCommand().empty())
		return ;
	std::map<std::string, CommandHandler>::iterator	it;

	it = _commandMap.find(cmd.getCommand());
	if (it != _commandMap.end())
		(this->*(it->second))(client, cmd);
	else
		client->queueWrite(ERR_UNKNOWNCOMMAND(_serverName, client->getNick(), cmd.getCommand()));
}

void	Server::handlePass(Client *client, const Command &cmd)
{
	if (client->registered_ == true)
	{
		client->queueWrite(ERR_ALREADYREGISTERED(_serverName, client->getNick()));
		return ;
	}
	if (cmd.getParams().size() < 1)
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		return ;
	}
	if (cmd.getParams()[0] != _password)
	{
		client->queueWrite(ERR_PASSWDMISMATCH(_serverName, client->getNick()));
		markClosing(client);
		return ;
	}
	client->passed_ = true;
}

void	Server::handlePing(Client *client, const Command &)
{
	if (client->registered_ == false)
	{
		client->queueWrite(ERR_NOTREGISTERED(_serverName, client->getNick()));
		return ;
	}
	client->queueWrite("Pong!");
}
