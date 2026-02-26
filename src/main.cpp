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

int	main(int argc, char **argv)
{
	Server	server(4444, "4444");

	if (argc != 3)
	{
		std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return (1);
	}
	try
	{
		signal(SIGINT, Server::signalHandler);
		signal(SIGQUIT, Server::signalHandler);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	std::cout << "Server closed!" << std::endl;
}