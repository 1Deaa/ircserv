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

class Client : public Socket
{
	private:
		std::string	_writeBuffer;
		std::string	_readBuffer;
		std::string	_ipAddress;
		bool		_closing;
	public:
		Client(fd_t fd);
		void				setIPAddress(const std::string &);
		const std::string	&getIPAddress(void) const;
		std::string			&getReadBuffer(void);
		std::string			&getWriteBuffer(void);
		void				updatePollEvents(void);
		void				queueWrite(std::string);
		void				setClosing(bool);
		bool				closingStatus(void);
		~Client();
};

#endif