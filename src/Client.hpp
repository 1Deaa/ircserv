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

enum CLIENT_NETWORK_STATE
{
	RUNNING,
	CLOSING,
	DISCONN
};

enum CLIENT_LOGIN_STATE
{
	LOGIN_PASS = 1 << 0,
	LOGIN_NICK = 1 << 1,
	LOGIN_USER = 1 << 2,
	LOGIN_REGS = 1 << 3
};

class Client : public Socket
{
	private:
		std::string				_writeBuffer;
		std::string				_readBuffer;
		std::string				_ipAddress;
		std::string				_nickName;
		CLIENT_NETWORK_STATE	_networkState;
		int						_loginState;
	public:
		Client(fd_t fd);
		void						setIPAddress(const std::string &);
		const std::string			&getIPAddress(void) const;
		std::string					&getReadBuffer(void);
		std::string					&getWriteBuffer(void);
		void						updatePollEvents(void);
		void						queueWrite(std::string);
		void						setNetworkState(CLIENT_NETWORK_STATE);
		const CLIENT_NETWORK_STATE	&getNetworkState(void) const;
		const std::string			getNick(void) const;
		void						setNick(const std::string &);
		void						addLoginState(int);
		void						rmvLoginState(int);
		int							getLoginState(void) const;
		bool						hasLoginState(int) const;
		~Client();
};

#endif