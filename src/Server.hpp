/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 19:27:23 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 19:27:23 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

# include <iostream>
# include <vector>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <unistd.h>
# include <arpa/inet.h>
# include <poll.h>
# include <signal.h>
# include "Client.hpp"
# include "Socket.hpp"

class Server
{
	private:
		int						_port;
		static bool				_signal;
		std::string				_password;
		Socket*					_selfSocket;
		std::vector<Socket*>	_sockets;
		void	selfSocket(void);
		void	acceptClient(void);
		void	receiveData(Client*);
	public:
		Server(int port, std::string password);
		~Server();
		void		run(void);
		static void	signalHandler(int signum);
};

#endif