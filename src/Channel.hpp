/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/07 12:40:04 by drahwanj          #+#    #+#             */
/*   Updated: 2026/03/07 12:40:05 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

# include "Client.hpp"
# include <vector>

class Client;

class Channel
{
	private:
		std::string				_name;
		std::vector<Client*>	_members;
		std::vector<Client*>	_operators;
		std::vector<Client*>	_invited;
		std::string				_topic;
		bool					_inviteOnly;
		bool					_topicRestricted;
		std::string				_key;
		size_t					_userLimit;
	public:
		Channel(const std::string &);
		~Channel();
	public:
		const std::string			&getName(void) const;
		const std::string			&getTopic(void) const;
		const std::vector<Client*>	&getMembers(void) const;
		const std::vector<Client*>	&getOperators(void) const;
	public:
		void	setName(const std::string &);
		void	setTopic(const std::string &);
		void	setInviteOnly(bool);
		void	setTopicRestricted(bool);
		void	setUserLimit(size_t);
		void	setKey(const std::string &);
	public:
		void	addMember(Client *);
		void	addOperator(Client *);
		void	addInvite(Client *);
	public:
		bool	isMember(Client *) const;
		bool	isOperator(Client *) const;
		bool	isInviteOnly() const;
		bool	isTopicRestricted() const;
		bool	isInvited(Client *) const;
		bool	hasKey() const;
		bool	hasLimit() const;
	public:
		const std::string	&getKey() const;
		const size_t		&getUserLimit() const;
	public:
		void	removeMember(Client *);
		void	removeOperator(Client *);
		void	removeKey();
		void	removeUserLimit();
		void	removeInvite(Client *);
};

#endif
