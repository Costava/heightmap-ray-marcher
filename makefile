# makefile

build: hmap

clean:
	rm -f ./hmap
	rm -f ./tmp/stb_image.o
	rm -f ./tmp/stb_image_write.o

tmp:
	mkdir tmp

tmp/stb_image.o: vendor/stb_image.c vendor/stb_image.h | tmp
	gcc --output $@ -std=c99 -c -w vendor/stb_image.c

tmp/stb_image_write.o: vendor/stb_image_write.c vendor/stb_image_write.h | tmp
	gcc --output $@ -std=c99 -c -w vendor/stb_image_write.c

hmap: main/hmap.cpp src/* vendor/* tmp/stb_image.o tmp/stb_image_write.o
	g++ --output $@ -std=c++98 -Wall -Wextra -Wconversion -g                   \
	-I ./src -I ./vendor                                                       \
	-lSDL2 -lSDL2_ttf -lGL                                                     \
	main/hmap.cpp src/*.cpp tmp/stb_image.o tmp/stb_image_write.o
