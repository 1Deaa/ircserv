/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 19:27:32 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 19:27:33 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Socket.hpp"

enum CLIENT_STATE
{
	RUNNING,
	CLOSING,
	DISCONN
};

class Client : public Socket
{
	private:
		std::string		_writeBuffer;
		std::string		_readBuffer;
		std::string		_ipAddress;
		CLIENT_STATE	_state;
	public:
		Client(fd_t fd);
		void				setIPAddress(const std::string &);
		const std::string	&getIPAddress(void) const;
		std::string			&getReadBuffer(void);
		std::string			&getWriteBuffer(void);
		void				updatePollEvents(void);
		void				queueWrite(std::string);
		void				setState(CLIENT_STATE);
		const CLIENT_STATE	&getState(void) const;
		~Client();
};

#endif