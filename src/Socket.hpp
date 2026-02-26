/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 19:34:34 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 19:34:34 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
#define SOCKET_HPP

# include <poll.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdexcept>

typedef int	fd_t;

enum SocketType
{
	SERVER,
	CLIENT
};

class Socket
{
	private:
		SocketType	_type;
		pollfd		_prq;
	public:
		Socket(fd_t, SocketType);
		virtual ~Socket();
		SocketType	getType(void) const;
		fd_t		getFd(void) const;
		short		getEvents(void) const;
		short		getRevents(void) const;
		pollfd		&getPollFd(void);
		void		setEvents(short);
		void		setRevents(short);
		void		setFd(fd_t);
		void		closeFd(void);
		void		addEvent(short);
		void		removeEvent(short);
		bool		hasRevent(short) const;
		static void	setNonBlocking(Socket *);
		static void	setNonBlocking(fd_t);
};

#endif
