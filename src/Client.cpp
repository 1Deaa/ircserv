/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 19:27:37 by drahwanj          #+#    #+#             */
/*   Updated: 2026/02/26 19:27:38 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(fd_t fd) : Socket(fd, CLIENT) {}
Client::~Client() {}

void	Client::setIPAddress(const std::string &ip)
{
	_ipAddress = ip;
}

const std::string	&Client::getIPAddress(void) const
{
	return (_ipAddress);
}
