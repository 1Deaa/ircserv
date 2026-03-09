/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:57:41 by drahwanj          #+#    #+#             */
/*   Updated: 2026/03/04 19:57:42 by drahwanj         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
#define COMMAND_HPP

# include <string>
# include <vector>
# include <iostream>

class Command
{
	private:
		std::string					_command;
		std::vector<std::string>	_params;
		std::string					_trailing;
		void		parse(const std::string &);
		void		print(void) const;
	public:
		Command(const std::string &);
		const std::string				&getCommand(void) const;
		const std::vector<std::string>	&getParams(void) const;
		const std::string				&getTrailing(void) const;
		static std::string				toUpper(std::string);
};

#endif