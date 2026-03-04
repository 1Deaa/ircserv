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

SRC=main.cpp \
	Server.cpp Client.cpp \
	Socket.cpp Command.cpp Execute.cpp

SOURCES=$(addprefix src/, $(SRC))

OBJ=$(SOURCES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME) -I src/

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re