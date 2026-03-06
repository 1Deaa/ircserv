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

# define CRLF	"\r\n"
# define RED	"\e[0;31m"
# define WHITE	"\e[0;37m"
# define GREEN	"\e[0;32m"
# define YELLOW	"\e[0;33m"
# define BLUE	"\e[0;34m"
# define RESET	"\e[0m"

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
# include <cstring>
# include <algorithm>
# include "Client.hpp"
# include "Socket.hpp"
# include "Command.hpp"
# include "Replies.hpp"
# include <algorithm>
# include <map>

enum LOG
{
	CONNLOG,
	NORMLOG,
	ERRLOG,
	DISCLOG
};

class Server
{
	private:
		int						_port;
		static bool				_signal;
		std::string				_serverName;
		std::string				_password;
		Socket*					_selfSocket;
		std::vector<Socket*>	_sockets;
		std::vector<Socket*>	_disconnected;
		void	selfSocket(void);
		void	acceptClient(void);
		void	receiveData(Client*);
		void	sendData(Client*);
		void	markDisconnected(Client*);
		void	markClosing(Client*);
		void	disconnectSocket(Socket *);
		void	processBuffer(Client*);
		//
		void	executeCommand(Client*, const Command &);
		void	handlePass(Client*, const Command &);
		void	handlePing(Client*, const Command &);
		void	handleNick(Client*, const Command &);
		typedef void (Server::*CommandHandler)(Client*, const Command &);
		std::map<std::string, CommandHandler>	_commandMap;
		bool	nickExists(Client*, const std::string &);
		void	tryRegister(Client*);
	public:
		Server(int, const std::string &);
		~Server();
		void		run(void);
		static void	printError(const std::string &);
		static void	printClientLog(Client *client, LOG type, const std::string &msg);
		static void	printClientLog(Client *client, LOG type);
		static void	signalHandler(int);
};

#endif