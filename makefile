# makefile

build: hmap

clean:
	rm -f ./hmap
	rm -f ./tmp/stb_image_write.o

tmp:
	mkdir tmp

tmp/stb_image_write.o: vendor/stb_image_write.c vendor/stb_image_write.h | tmp
	g++ --output $@ -c -w vendor/stb_image_write.c

hmap: main/main.cpp src/*.cpp src/*.hpp vendor/* tmp/stb_image_write.o
	g++ -std=c++98 -Wall -Wextra -Wconversion --output $@                      \
	-I ./src -I ./vendor                                                       \
	-lSDL2 -lGL                                                                \
	main/main.cpp src/*.cpp tmp/stb_image_write.o
