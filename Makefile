CC=g++
NAME=server

all: $(NAME)
	g++ -o $(NAME).exe $(NAME).cpp -lstdc++ -std=c++98 -W -Wall
