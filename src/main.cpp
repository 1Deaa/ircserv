/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 22:37:18 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 22:37:18 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static bool	validatePassword(const std::string &pass)
{
	if (pass.empty())
		return (false);
	if (pass.size() > 20)
		return (false);
	return (true);
}

static bool	validatePort(const std::string &port)
{
	if (port.empty())
		{Server::printError("port cannot be empty!"); return (false);}
	if (port.find_first_not_of("0123456789") != std::string::npos)
		{Server::printError("port must be a number!"); return (false);}
	if (port.size() > 1 && port[0] == '0')
		{Server::printError("port cannot have leading zeros!"); return (false);}
	if (port.size() > 5)
		{Server::printError("port cannot exceed 5 digits!"); return (false);}
	int	portVal = std::atoi(port.c_str());
	if (portVal < 1024 || portVal > 65535)
		{Server::printError("port must be between 1024 and 65535!"); return (false);}
	return (true);
}

static bool	validateArgs(const int argc, const char **argv)
{
	if (argc != 3)
	{
		std::cout << "ircserv: usage " << argv[0] << " <port> <password>" << std::endl;
		return (false);
	}
	if (!validatePort(argv[1]))
		return (false);
	if (!validatePassword(argv[2]))
		return (false);
	return (true);
}

int	main(const int argc, const char **argv)
{
	if (!validateArgs(argc, argv))
		return (1);
	Server	server(std::atoi(argv[1]), argv[2]);
	try
	{
		signal(SIGINT, Server::signalHandler);
		signal(SIGQUIT, Server::signalHandler);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "ircserv: " << e.what() << std::endl;
	}
	std::cout << "Server closed!" << std::endl;
}