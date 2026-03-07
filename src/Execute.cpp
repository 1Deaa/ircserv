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
	client->setPassword(pass);
	client->addLoginState(LOGIN_PASS);
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
	if (nick.empty() || nick.length() > 9)
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
		printClientLog(client, ERRLOG, "entered an erroneous nickname.");
		return ;
	}
	if (client->getNick() == nick)
		return ;
	if (nickExists(client, nick))
	{
		client->queueWrite(ERR_NICKNAMEINUSE(_serverName, client->getNick(), nick));
		return ;
	}
	if (client->hasLoginState(LOGIN_REGS))
	{
		std::string	msg = ":" + client->getPrefix() + " NICK :" + nick;
		broadcast(client, msg);
	}
	client->setNick(nick);
	client->addLoginState(LOGIN_NICK);
	printClientLog(client, NORMLOG, "set a nickname (" + nick + ").");
	tryRegister(client);
}

void	Server::handleUser(Client *client, const Command &cmd)
{
	if (client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_ALREADYREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "already registered.");
		return ;
	}
	if (client->hasLoginState(LOGIN_USER))
	{
		client->queueWrite(ERR_ALREADYREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "already set a username (" + client->getUser() + ":" + client->getRealName() + ").");
		return ;
	}
	if (cmd.getParams().size() < 3 || cmd.getTrailing().empty())
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		printClientLog(client, ERRLOG, cmd.getCommand() + " missing parameter.");
		return ;
	}

	std::string	username = cmd.getParams()[0]; // TODO: validate username length
	std::string	realname = cmd.getTrailing();

	client->setUser(username);
	client->setRealName(realname);
	printClientLog(client, NORMLOG, "set a username (" + client->getUser() + ":" + client->getRealName() + ").");
	client->addLoginState(LOGIN_USER);
	tryRegister(client);
}

void	Server::handleQuit(Client *client)
{
	disconnectClient(client, "Client Quit");
	markDisconnected(client);
}

void	Server::handleQuit(Client *client, const Command &cmd)
{
	std::string	reason = "Client Quit";
	if (!cmd.getTrailing().empty())
		reason = cmd.getTrailing();
	disconnectClient(client, reason);
	client->getWriteBuffer().clear();
	markDisconnected(client);
}

void	Server::disconnectClient(Client *client, const std::string &reason)
{
	std::string	msg = ":" + client->getPrefix() + " QUIT :" + reason;

	broadcast(client, msg);

	const std::vector<Channel*>	channels = client->getChannels();

	for (std::size_t i = 0; i < channels.size(); ++i)
	{
		Channel	*channel = channels[i];

		channel->removeMember(client);
		client->rmvChannel(channel);

		if (channel->getMembers().empty())
			removeChannel(channel);
	}
}

static bool	isValidChannelName(const std::string &str)
{
	if (str.empty() || str[0] != '#')
		return (false);
	if (str.length() > 50)
		return (false);
	for (size_t i = 1; i < str.length(); ++i)
	{
		if (str[i] == ' ' || str[i] == ',' || str[i] == '\r' || str[i] == '\n')
			return (false);
	}
	return (true);
}

void	Server::handleJoin(Client *client, const Command &cmd)
{
	if (!client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_NOTREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "has not registered.");
		return ;
	}
	std::string	channelName;
	if (!cmd.getParams().empty())
		channelName = cmd.getParams()[0];
	else if (!cmd.getTrailing().empty())
		channelName = cmd.getTrailing();
	else
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		return ;
	}

	if (!isValidChannelName(channelName))
	{
		client->queueWrite(ERR_NOSUCHCHANNEL(_serverName, client->getNick(), channelName));
		return ;
	}
	
	if (client->getChannels().size() >= MAX_CHANNELS_PER_CLIENT)
	{
		client->queueWrite(ERR_TOOMANYCHANNELS(_serverName, client->getNick(), channelName));
		return ;
	}

	Channel	*channel = getChannel(channelName);

	if (!channel)
	{
		channel = createChannel(channelName);
		channel->addMember(client);
		channel->addOperator(client);
		client->addChannel(channel);
	}
	else
	{
		if (channel->isMember(client))
			return ;
		client->addChannel(channel);
		channel->addMember(client);
	}
	broadcast(channel, ":" + client->getPrefix() + " JOIN " + channel->getName());
	Server::printClientLog(client, NORMLOG, "joined channel " + channel->getName());
}

void	Server::handlePrivmsg(Client *client, const Command &cmd)
{
	if (!client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_NOTREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "has not registered.");
		return ;
	}

	if (cmd.getParams().empty())
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		return ;
	}

	if (cmd.getTrailing().empty())
	{
		client->queueWrite(ERR_NOTEXTTOSEND(_serverName, client->getNick()));
		return ;
	}

	std::string	target = cmd.getParams()[0];
	std::string	message = cmd.getTrailing();
	std::string	msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message;
	
	if (target[0] == '#')
	{
		Channel	*channel = getChannel(target);

		if (!channel)
		{
			client->queueWrite(ERR_NOSUCHCHANNEL(_serverName, client->getNick(), target));
			return ;
		}
		if (!channel->isMember(client))
		{
			client->queueWrite(ERR_CANNOTSENDTOCHAN(_serverName, client->getNick(), target));
			return ;
		}
		broadcast(channel, msg, client);
	}
	else
	{
		Client	*targetClient = getClient(target);

		if (!targetClient)
		{
			client->queueWrite(ERR_NOSUCHNICK(_serverName, client->getNick(), target));
			return ;
		}
		targetClient->queueWrite(msg);
	}
}

void	Server::handlePart(Client *client, const Command &cmd)
{
	if (!client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_NOTREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "has not registered.");
		return ;
	}

	if (cmd.getParams().empty())
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		return ;
	}

	std::string	channelName = cmd.getParams()[0];
	Channel	*channel = getChannel(channelName);

	if (!channel)
	{
		client->queueWrite(ERR_NOSUCHCHANNEL(_serverName, client->getNick(), channelName));
		return ;
	}

	std::string	msg = ":" + client->getPrefix() + " PART " + channelName;

	if (!cmd.getTrailing().empty())
		msg += " :" + cmd.getTrailing();
	printClientLog(client, NORMLOG, "left channel " + channel->getName());
	broadcast(channel, msg);
	channel->removeMember(client);
	client->rmvChannel(channel);
	if (channel->getMembers().empty())
		removeChannel(channel);
}

void	Server::handleKick(Client *client, const Command &cmd)
{
	if (!client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_NOTREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "has not registered.");
		return ;
	}
	if (cmd.getParams().size() < 2)
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		return ;
	}
	std::string	channelName = cmd.getParams()[0];
	std::string	targetNick = cmd.getParams()[1];

	Channel	*channel = getChannel(channelName);
	if (!channel)
	{
		client->queueWrite(ERR_NOSUCHCHANNEL(_serverName, client->getNick(), channelName));
		return ;
	}
	if (!channel->isMember(client))
	{
		client->queueWrite(ERR_NOTONCHANNEL(_serverName, client->getNick(), channelName));
		return ;
	}
	if (!channel->isOperator(client))
	{
		client->queueWrite(ERR_CHANOPRIVSNEEDED(_serverName, client->getNick(), channelName));
		return ;
	}
	Client	*target = getClient(targetNick);
	if (!target || !channel->isMember(target))
	{
		client->queueWrite(ERR_USERNOTINCHANNEL(_serverName, client->getNick(), targetNick, channelName));
		return ;
	}
	std::string	reason = targetNick;
	if (!cmd.getTrailing().empty())
		reason = cmd.getTrailing();
	std::string	msg = ":" + client->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason;
	broadcast(channel, msg);
	channel->removeMember(target);
	target->rmvChannel(channel);
	if (channel->getMembers().empty())
		removeChannel(channel);
}