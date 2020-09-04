# makefile

CC:=g++
CFLAGS:=-std=c++98 -Wall
SRCDIR:=./src
OBJDIR:=./obj

# Recipe for building what will be a dependency of the main executable
BUILD_DEP=$(CC) $(CFLAGS) -c --output $@ $<

init:
	mkdir -p $(OBJDIR)

build: hmap

buildfresh: clean build

clean:
	rm -f hmap
	rm -f $(OBJDIR)/*.o

hmap: $(SRCDIR)/main.cpp \
	$(OBJDIR)/AABB.o \
	$(OBJDIR)/Orthographic.o \
	$(OBJDIR)/Perspective.o \
	$(OBJDIR)/Spherical.o
	$(CC) $(CFLAGS) --output $@ -lSDL2 -lGL $^

################################################################################

$(OBJDIR)/AABB.o: $(SRCDIR)/AABB.cpp $(SRCDIR)/AABB.hpp
	$(BUILD_DEP)

$(OBJDIR)/Orthographic.o: $(SRCDIR)/Orthographic.cpp $(SRCDIR)/Orthographic.hpp
	$(BUILD_DEP)

$(OBJDIR)/Perspective.o: $(SRCDIR)/Perspective.cpp $(SRCDIR)/Perspective.hpp
	$(BUILD_DEP)

$(OBJDIR)/Spherical.o: $(SRCDIR)/Spherical.cpp $(SRCDIR)/Spherical.hpp
	$(BUILD_DEP)
