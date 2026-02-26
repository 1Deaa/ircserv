# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: drahwanj <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/26 19:19:14 by drahwanj          #+#    #+#              #
#    Updated: 2026/02/26 19:19:15 by drahwanj         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME=ircserv

CXX=c++
CXXFLAGS=-Wall -Wextra -Werror -std=c++98

SRC=main.cpp
SOURCES=$(addprefix src/, $(SRC))

# INC=
# INCLUDES=$(addprefix -I src/, $(INC))

OBJ=$(SOURCES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re