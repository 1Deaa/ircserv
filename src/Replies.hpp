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
// ACK
# define RPL_WELCOME

#endif
