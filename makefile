# makefile

build: hmap

clean:
	rm -f ./hmap

hmap: ./main/main.cpp ./src/*.cpp ./src/*.hpp ./vendor/*
	g++ -std=c++98 -Wall -Wextra -Wconversion --output $@                      \
	-I ./src -I ./vendor                                                       \
	-lSDL2 -lGL                                                                \
	./main/main.cpp ./src/*.cpp
