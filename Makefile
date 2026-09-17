# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mhend <mhend@student.1337.ma>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/08/22 00:00:00 by mhend             #+#    #+#              #
#    Updated: 2026/09/05 20:04:51 by mhend            ###   ########.ma        #
#                                                                              #
# **************************************************************************** #

NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread
INCLUDES	= -I include

SRC_DIR		= src
OBJ_DIR		= obj

SRCS		= main.c parse.c time.c log.c pqueue.c pqueue2.c pqueue3.c \
			  wake.c dongle.c dongle2.c arbiter.c request.c coder.c \
			  monitor.c init.c run.c

OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))
HEADER		= include/codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER) | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
