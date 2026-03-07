/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 21:02:30 by drahwanj          #+#    #+#             */
/*   Updated: 2026/03/04 21:02:31 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLIES_HPP
#define REPLIES_HPP

// ERR
# define ERR_UNKNOWNCOMMAND(serverName, nickName, command) (":" + serverName + " 421 " + nickName + " " + command + " :Unknown command")
# define ERR_ALREADYREGISTERED(serverName, nickName) (":" + serverName + " 462 " + nickName + " :You may not reregister")
# define ERR_NEEDMOREPARAMS(serverName, nickName, command) (":" + serverName + " 461 " + nickName + " " + command + " :Not enough parameters")
# define ERR_PASSWDMISMATCH(serverName, nickName) (":" + serverName + " 464 " + nickName + " :Password incorrect")
# define ERR_NOTREGISTERED(serverName, nickName) (":" + serverName + " 451 " + nickName + " :You have not registered")
# define ERR_ERRONEUSNICKNAME(serverName, nickName, tryName) (":" + serverName + " 432 " + nickName + " " + tryName + " :Erroneous nickname")
# define ERR_NICKNAMEINUSE(serverName, nickName, tryName) (":" + serverName + " 433 " + nickName + " " + tryName + " :Nickname is already in use")
# define ERR_NOSUCHCHANNEL(serverName, nickName, channelName) (":" + serverName + " 403 " + nickName + " " + channelName + " :No such channel")
# define ERR_TOOMANYCHANNELS(serverName, nickName, channelName) (":" + serverName + " 405 " + nickName + " " + channelName + " :You have joined too many channels")
# define ERR_NOSUCHNICK(serverName, nickName, target) (":" + serverName + " 401 " + nickName + " " + target + " :No such nick/channel")
# define ERR_NOTEXTTOSEND(serverName, nickName) (":" + serverName + " 412 " + nickName + " :No text to send")
# define ERR_NOTONCHANNEL(serverName, nickName, channelName) (":" + serverName + " 442 " + nickName + " " + channelName + " :You're not on that channel")
# define ERR_CANNOTSENDTOCHAN(serverName, nickName, channelName) (":" + serverName + " 404 " + nickName + " " + channelName + " :Cannot send to channel")
# define ERR_USERNOTINCHANNEL(serverName, nick, target, channel) (":" + serverName + " 441 " + nick + " " + target + " " + channel + " :They aren't on that channel")
# define ERR_CHANOPRIVSNEEDED(serverName, nick, channel) (":" + serverName + " 482 " + nick + " " + channel + " :You're not channel operator")
// RPL
# define RPL_WELCOME(serverName, nickName, userName, hostName) (":" + serverName + " 001 " + nickName + " :Welcome to the Internet Relay Network " + nickName + "!" + userName + "@" + hostName)

#endif
