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
	if (client->hasLoginState(LOGIN_PASS))
	{
		client->queueWrite(ERR_ALREADYREGISTERED(_serverName, client->getNick()));
		return;
	}
	client->setPassword(pass);
	client->addLoginState(LOGIN_PASS);
	tryRegister(client);
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
		client->queueWrite(msg);
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

		bool	wasOperator = channel->isOperator(client);
		
		channel->removeMember(client);
		client->rmvChannel(channel);

		if (wasOperator && channel->getOperators().empty() && !channel->getMembers().empty())
		{
			Client	*newOp = channel->getMembers()[0];
			channel->addOperator(newOp);
			std::string	modeMsg = ":" + newOp->getNick() + " MODE " + channel->getName() + " +o " + newOp->getNick();
			broadcast(channel, modeMsg);
		}

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
		if (channel->isInviteOnly() && !channel->isInvited(client))
		{
			client->queueWrite(ERR_INVITEONLYCHAN(_serverName, client->getNick(), channelName));
			return ;
		}
		if (channel->hasKey())
		{
			if (cmd.getParams().size() < 2 || cmd.getParams()[1] != channel->getKey())
			{
				client->queueWrite(ERR_BADCHANNELKEY(_serverName, client->getNick(), channel->getName()));
				return ;
			}
		}
		if (channel->hasLimit() && channel->getMembers().size() >= channel->getUserLimit())
		{
			client->queueWrite(ERR_CHANNELISFULL(_serverName, client->getNick(), channelName));
			return ;
		}
		client->addChannel(channel);
		channel->addMember(client);
		channel->removeInvite(client);
	}
	broadcast(channel, ":" + client->getPrefix() + " JOIN " + channel->getName());
	std::string	names = buildNames(channel);
	if (channel->getTopic().empty())
		client->queueWrite(RPL_NOTOPIC(_serverName, client->getNick(), channel->getName()));
	else
		client->queueWrite(RPL_TOPIC(_serverName, client->getNick(), channel->getName(), channel->getTopic()));
	client->queueWrite(RPL_NAMREPLY(_serverName, client->getNick(), channel->getName(), names));
	client->queueWrite(RPL_ENDOFNAMES(_serverName, client->getNick(), channel->getName()));
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
	if (!channel->isMember(client))
	{
		client->queueWrite(ERR_NOTONCHANNEL(_serverName, client->getNick(), channelName));
		return ;
	}
	std::string	msg = ":" + client->getPrefix() + " PART " + channelName;

	if (!cmd.getTrailing().empty())
		msg += " :" + cmd.getTrailing();
	printClientLog(client, NORMLOG, "left channel " + channel->getName());
	broadcast(channel, msg);

	bool	wasOperator = channel->isOperator(client);

	channel->removeMember(client);
	client->rmvChannel(channel);

	if (wasOperator && channel->getOperators().empty() && !channel->getMembers().empty())
	{
		Client	*newOp = channel->getMembers()[0];
		channel->addOperator(newOp);
		std::string	modeMsg = ":" + newOp->getNick() + " MODE " + channel->getName() + " +o " + newOp->getNick();
		broadcast(channel, modeMsg);
	}

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
	std::string	msg = ":" + client->getPrefix() + " KICK " + channelName + " " + target->getNick() + " :" + reason;
	broadcast(channel, msg);

	bool	wasOperator = channel->isOperator(client);

	channel->removeMember(target);
	target->rmvChannel(channel);

	if (wasOperator && channel->getOperators().empty() && !channel->getMembers().empty())
	{
		Client	*newOp = channel->getMembers()[0];
		channel->addOperator(newOp);
		std::string	modeMsg = ":" + newOp->getNick() + " MODE " + channel->getName() + " +o " + newOp->getNick();
		broadcast(channel, modeMsg);
	}
	if (channel->getMembers().empty())
		removeChannel(channel);
}

void	Server::handleTopic(Client *client, const Command &cmd)
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

	if (!channel->isMember(client))
	{
		client->queueWrite(ERR_NOTONCHANNEL(_serverName, client->getNick(), channelName));
		return ;
	}

	if (cmd.getTrailing().empty())
	{
		if (channel->getTopic().empty())
		{
			client->queueWrite(RPL_NOTOPIC(_serverName, client->getNick(), channelName));
		}
		else
		{
			client->queueWrite(RPL_TOPIC(_serverName, client->getNick(), channelName, channel->getTopic()));
		}
		return ;
	}
	if (channel->isTopicRestricted() && !channel->isOperator(client))
	{
		client->queueWrite(ERR_CHANOPRIVSNEEDED(_serverName, client->getNick(), channel->getName()));
		return ;
	}
	std::string	topic = cmd.getTrailing();
	channel->setTopic(topic);
	std::string	msg = ":" + client->getPrefix() + " TOPIC " + channelName + " :" + topic;
	broadcast(channel, msg);
}

void	Server::handleInvite(Client *client, const Command &cmd)
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

	std::string	targetNick = cmd.getParams()[0];
	std::string	channelName = cmd.getParams()[1];

	Client *target = getClient(targetNick);

	if (!target)
	{
		client->queueWrite(ERR_NOSUCHNICK(_serverName, client->getNick(), targetNick));
		return ;
	}
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

	if (channel->isMember(target))
	{
		client->queueWrite(ERR_USERONCHANNEL(_serverName, client->getNick(), targetNick, channelName));
		return ;
	}
	client->queueWrite(RPL_INVITING(_serverName, client->getNick(), targetNick, channelName));
	channel->addInvite(target);
	std::string msg = ":" + client->getPrefix() + " INVITE " + targetNick + " " + channelName;
	target->queueWrite(msg);
}

static void	sendChannelModes(Client *client, Channel *channel, const std::string _serverName)
{
	std::string	modes = "+";

	if (channel->isInviteOnly())
		modes += "i";
	if (channel->isTopicRestricted())
		modes += "t";
	if (!channel->getKey().empty())
		modes += "k";
	if (channel->getUserLimit() > 0)
		modes += "l";

	client->queueWrite(":" + _serverName + " 324 " + client->getNick() + " " + channel->getName() + " " + modes);
}

void	Server::handleMode(Client *client, const Command &cmd)
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

	if (cmd.getParams().size() == 1)
	{
		sendChannelModes(client, channel, _serverName);
		return ;
	}

	if (!channel->isOperator(client))
	{
		client->queueWrite(ERR_CHANOPRIVSNEEDED(_serverName, client->getNick(), channelName));
		return ;
	}
	parseModeChange(client, channel, cmd);
}

void	Server::parseModeChange(Client *client, Channel *channel, const Command &cmd)
{
	std::string	modeString = cmd.getParams()[1];
	bool	adding = true;
	size_t	paramIndex = 2;

	for (size_t i = 0; i < modeString.size(); i++)
	{
		char mode = modeString[i];

		if (mode == '+')
		{
			adding = true;
			continue ;
		}
		if (mode == '-')
		{
			adding = false;
			continue ;
		}
		switch (mode)
		{
			case 'i': handleModeInvite(client, channel, adding); break ;
			case 't': handleModeTopic(client, channel, adding); break ;
			case 'k': handleModeKey(client, channel, adding, cmd, paramIndex); break ;
			case 'l': handleModeLimit(client, channel, adding, cmd, paramIndex); break;
			case 'o': handleModeOperator(client, channel, adding, cmd, paramIndex); break;
			default: break ;
		}
	}
}

void	Server::handleModeInvite(Client *client, Channel *channel, bool adding)
{
	if (channel->isInviteOnly() == adding)
		return ;
	channel->setInviteOnly(adding);
	std::string	msg = ":" + client->getPrefix() + " MODE " + channel->getName() + (adding ? " +i" : " -i");
	broadcast(channel, msg);
}

void	Server::handleModeTopic(Client *client, Channel *channel, bool adding)
{
	if (channel->isTopicRestricted() == adding)
		return ;
	channel->setTopicRestricted(adding);
	std::string	msg = ":" + client->getPrefix() + " MODE " + channel->getName() + (adding ? " +t" : " -t");
	broadcast(channel, msg);
}

void	Server::handleModeOperator(Client *client, Channel *channel, bool adding, const Command &cmd, size_t &paramIndex)
{
	if (paramIndex >= cmd.getParams().size())
	{
		client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
		return ;
	}
	std::string	targetNick = cmd.getParams()[paramIndex++];
	Client	*target = getClient(targetNick);

	if (!target)
	{
		client->queueWrite(ERR_NOSUCHNICK(_serverName, client->getNick(), targetNick));
		return ;
	}

	if (!channel->isMember(target))
	{
		client->queueWrite(ERR_USERNOTINCHANNEL(_serverName, client->getNick(), targetNick, channel->getName()));
		return ;
	}
	bool	changed = false;
	if (adding && !channel->isOperator(target))
	{
		channel->addOperator(target);
		changed = true;
	}
	else if (!adding && channel->isOperator(target))
	{
		channel->removeOperator(target);
		changed = true;
	}
	if (changed)
	{
		std::string	msg = ":" + client->getPrefix() + " MODE " + channel->getName() + (adding ? " +o " : " -o ") + targetNick;
		broadcast(channel, msg);
	}
}

void	Server::handleModeKey(Client *client, Channel *channel, bool adding, const Command &cmd, size_t &paramIndex)
{
	if (adding)
	{
		if (paramIndex >= cmd.getParams().size())
		{
			client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
			return ;
		}

		std::string	key = cmd.getParams()[paramIndex++];
		if (channel->hasKey())
			return ;
		channel->setKey(key);
		std::string	msg = ":" + client->getPrefix() + " MODE " + channel->getName() + " +k " + key;
		broadcast(channel, msg);
	}
	else
	{
		if (!channel->hasKey())
			return ;
		channel->removeKey();
		std::string	msg = ":" + client->getPrefix() + " MODE " + channel->getName() + " -k";
		broadcast(channel, msg);
	}
}

void	Server::handleModeLimit(Client *client, Channel *channel, bool adding, const Command &cmd, size_t &paramIndex)
{
	if (adding)
	{
		if (paramIndex >= cmd.getParams().size())
		{
			client->queueWrite(ERR_NEEDMOREPARAMS(_serverName, client->getNick(), cmd.getCommand()));
			return ;
		}
		char	*end;
		long	limit = std::strtol(cmd.getParams()[paramIndex++].c_str(), &end, 10);
		if (*end != '\0' || limit <= 0)
			return ;
		
		channel->setUserLimit(limit);

		std::ostringstream	oss;
		oss << limit;
		std::string	msg = ":" + client->getPrefix() + " MODE " + channel->getName() + " +l " + oss.str();
		broadcast(channel, msg);
	}
	else
	{
		if (!channel->hasLimit())
			return ;
		channel->removeUserLimit();
		std::string	msg = ":" + client->getPrefix() + " MODE " + channel->getName() + " -l";
		broadcast(channel, msg);
	}
}

void	Server::handleWho(Client *client, const Command &cmd)
{
	if (!client->hasLoginState(LOGIN_REGS))
	{
		client->queueWrite(ERR_NOTREGISTERED(_serverName, client->getNick()));
		printClientLog(client, ERRLOG, "has not registered.");
		return ;
	}

	if (cmd.getParams().empty())
	{
		client->queueWrite(RPL_ENDOFWHO(_serverName, client->getNick(), "*"));
		return ;
	}

	std::string	mask = cmd.getParams()[0];

	Channel	*channel = getChannel(mask);

	if (channel)
	{
		const std::vector<Client*>	&members = channel->getMembers();

		for (size_t i = 0; i < members.size(); ++i)
		{
			Client	*target = members[i];
			std::string	status = "H";

			if (channel->isOperator(target))
				status += "@";
			
			client->queueWrite(RPL_WHOREPLY(_serverName,
											client->getNick(),
											channel->getName(),
											target->getUser(),
											target->getIPAddress(),
											_serverName,
											target->getNick(),
											status,
											target->getRealName()
				)
			);
		}
		client->queueWrite(RPL_ENDOFWHO(_serverName, client->getNick(), channel->getName()));
		return ;
	}
	Client	*target = getClient(mask);
	if (target)
	{
		client->queueWrite(RPL_WHOREPLY(_serverName,
										client->getNick(),
										"*",
										target->getUser(),
										target->getIPAddress(),
										_serverName,
										target->getNick(),
										"H",
										target->getRealName()));
	}
	client->queueWrite(RPL_ENDOFWHO(_serverName, client->getNick(), mask));
}

std::string	Server::buildNames(Channel *channel)
{
	std::string	names;
	const std::vector<Client*>	&members = channel->getMembers();

	for (size_t i = 0; i < members.size(); ++i)
	{
		if (channel->isOperator(members[i]))
			names += "@";
		names += members[i]->getNick();
		if (i + 1 < members.size())
			names += " ";
	}
	return (names);
}