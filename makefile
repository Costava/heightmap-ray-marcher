# makefile

build: hmap

clean:
	rm -f ./hmap

hmap: ./main/main.cpp ./src/*.cpp ./src/*.hpp
	g++ -std=c++98 -Wall --output $@ -I ./src -lSDL2 -lGL \
	./main/main.cpp ./src/*.cpp
