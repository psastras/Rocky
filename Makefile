# Makefile for Rocky by psastras

CC = g++

ifeq ($(BUILD_TYPE), debug)
    CFLAGS = -Wall -g -std=c++0x
else
    CFLAGS = -O3 -msse2 -ffast-math -funswitch-loops -funsafe-math-optimizations \
    -fsingle-precision-constant -funsafe-loop-optimizations -fgcse-sm -fgcse-las -fsee -std=c++0x -static
endif


ifeq ($(OS),Windows_NT)
    LDFLAGS = -LC:/MinGW/msys/1.0/lib -lglew32 -lglu32 -lopengl32 -lgdi32 -LC:/MinGW/msys/1.0/local/lib -lfftw3f -lpthread -lIL
else
    LDFLAGS = -lm -lX11 -lGL -lGLU 
endif


SRCDIR := src
VSMLDIR := 3rdparty/VSML

INCLUDES = -IC:/MinGW/msys/1.0/include -IC:/MinGW/msys/1.0/local/include -I$(VSMLDIR)
OBJFILES := $(patsubst $(VSMLDIR)/%.cpp,$(VSMLDIR)/%.o,$(wildcard $(VSMLDIR)/*.cpp)) \
    $(patsubst $(SRCDIR)/%.cpp,$(SRCDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
COMPILE = $(CC) $(CFLAGS) $(INCLUDES) -c

all: water
water: $(OBJFILES)
	$(CC) $(LDFLAGS) -o bin/rocky.exe $(OBJFILES) $(LDFLAGS)


%.o: %.cpp
	$(COMPILE) -o $@ $<

.PHONY: clean
clean:
	rm -f $(SRCDIR)/*.o
