/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 20:00:54 by drahwanj          #+#    #+#             */
/*   Updated: 2026/03/04 20:00:54 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Command.hpp"

Command::Command(const std::string &cmd)
{
	parse(cmd);
	_command = toUpper(_command);
	print();
}

const std::string	&Command::getCommand(void) const
{
	return (_command);
}

const std::vector<std::string>	&Command::getParams(void) const
{
	return (_params);
}

const std::string	&Command::getTrailing(void) const
{
	return (_trailing);
}

void	Command::parse(const std::string &cmd) //<---- TODO: Fix parser too strict!
{
	std::string	tmp = cmd;
	size_t		pos;

	pos = tmp.find(' ');
	if (pos == std::string::npos)
	{
		_command = tmp;
		return ;
	}

	_command = tmp.substr(0, pos);
	tmp.erase(0, pos + 1);
	
	while (!tmp.empty())
	{
		if (tmp[0] == ':')
		{
			_trailing = tmp.substr(1);
			break ;
		}
		pos = tmp.find(' ');
		if (pos == std::string::npos)
		{
			_params.push_back(tmp);
			break ;
		}
		_params.push_back(tmp.substr(0, pos));
		tmp.erase(0, pos + 1);
		while (!tmp.empty() && tmp[0] == ' ')
			tmp.erase(0, 1);
	}
}

std::string	Command::toUpper(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
		str[i] = std::toupper(str[i]);
	return (str);
}

void	Command::print(void) const
{
	std::cout << "COMMAND: " << _command << std::endl;

	std::cout << "PARAMS:";
	for (size_t i = 0; i < _params.size(); ++i)
		std::cout << " " << _params[i];
	std::cout << std::endl;

	if (!_trailing.empty())
		std::cout << "TRAILING: " << _trailing << std::endl;
}