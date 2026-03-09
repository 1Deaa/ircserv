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

void Command::parse(const std::string &line)
{
	size_t i = 0;
	size_t len = line.length();

	// skip leading spaces
	while (i < len && line[i] == ' ')
		i++;

	// -------- COMMAND --------
	size_t start = i;
	while (i < len && line[i] != ' ')
		i++;

	_command = line.substr(start, i - start);

	// -------- PARAMETERS --------
	while (i < len)
	{
		// skip spaces
		while (i < len && line[i] == ' ')
			i++;

		if (i >= len)
			break;

		// trailing part
		if (line[i] == ':')
		{
			_trailing = line.substr(i + 1);
			break;
		}

		start = i;

		while (i < len && line[i] != ' ')
			i++;

		_params.push_back(line.substr(start, i - start));
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