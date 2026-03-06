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
	{
		client->queueWrite(ERR_UNKNOWNCOMMAND(_serverName, client->getNick(), cmd.getCommand()));
		printClientLog(client, ERRLOG, "sent an unknown command " + cmd.getCommand() + ".");
	}
}

void	Server::handlePass(Client *client, const Command &cmd)
{
	if (client->hasLoginState(LOGIN_PASS))
		return ;
	if (client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_ALREADYREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "already registered.");
		return ;
	}
	std::string	pass;

	if (!cmd.getParams().empty())
		pass = cmd.getParams()[0];
	else if (!cmd.getTrailing().empty())
		pass = cmd.getTrailing();
	else
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		printClientLog(client, ERRLOG, cmd.getCommand() + " missing parameter.");
		return ;
	}
	if (pass != _password)
	{
		client->queueWrite(ERR_PASSWDMISMATCH(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "entered incorrect password.");
		markClosing(client);
		return ;
	}
	client->addLoginState(LOGIN_PASS);
	printClientLog(client, NORMLOG, "entered correct password.");
	tryRegister(client);
}

void	Server::handlePing(Client *client, const Command &)
{
	if (!client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_NOTREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "has not registered.");
		return ;
	}
	client->queueWrite("Pong!");
}

static bool	isValidNick(const std::string &nick)
{
	if (nick.empty())
		return (false);
	if (!std::isalpha(nick[0]))
		return (false);
	for (size_t i = 1; i < nick.size(); i++)
	{
		if (!std::isalnum(nick[i]) && 
			nick[i] != '_' &&
			nick[i] != '-' &&
			nick[i] != '[' &&
			nick[i] != ']')
			return (false);
	}
	return (true);
}

void	Server::handleNick(Client *client, const Command &cmd)
{
	if (cmd.getParams().empty())
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		return ;
	}
	std::string	nick = cmd.getParams()[0];

	if (!isValidNick(nick))
	{
		client->queueWrite(ERR_ERRONEUSNICKNAME(_serverName, client->getNick(), nick));
		printClientLog(client, ERRLOG, "entered an invalid nickname.");
		return ;
	}
	if (client->getNick() == nick)
		return ;
	if (nickExists(client, nick))
	{
		client->queueWrite(ERR_NICKNAMEINUSE(_serverName, client->getNick(), nick));
		return ;
	}
	client->setNick(nick);
	client->addLoginState(LOGIN_NICK);
	printClientLog(client, NORMLOG, "set a nickname (" + nick + ").");
	tryRegister(client);
}