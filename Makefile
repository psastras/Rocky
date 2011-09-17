# Makefile for Rocky by psastras

CC = g++

ifeq ($(BUILD_TYPE), debug)
    CFLAGS = -Wall -g -std=c++0x
else
    CFLAGS = -O3 -msse2 -ffast-math -funswitch-loops -funsafe-math-optimizations \
    -fsingle-precision-constant -funsafe-loop-optimizations -fgcse-sm -fgcse-las -fsee -std=c++0x
endif



ifeq ($(OS),Windows_NT)
    EXECUTABLE = rocky.exe
    LDFLAGS = -LC:/MinGW/msys/1.0/lib -lglew32 -lglu32 -lopengl32 -lgdi32 -LC:/MinGW/msys/1.0/local/lib -lfftw3f -lpthread -lIL
    INCLUDES = -IC:/MinGW/msys/1.0/include -IC:/MinGW/msys/1.0/local/include -I$(VSMLDIR)
else
    EXECUTABLE = rocky
    LDFLAGS = -lm -lX11 -lGL -lGLU -L/contrib/projects/fftw/lib -lfftw3f -lfftw3f_threads -lpthread -lIL -L/contrib/projects/glew/lib -lGLEW -lrt
    INCLUDES = -I$(VSMLDIR) -I/contrib/projects/glew/include  -I/contrib/projects/fftw/include
endif


SRCDIR := src
VSMLDIR := 3rdparty/VSML


OBJFILES := $(patsubst $(VSMLDIR)/%.cpp,$(VSMLDIR)/%.o,$(wildcard $(VSMLDIR)/*.cpp)) \
    $(patsubst $(SRCDIR)/%.cpp,$(SRCDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
COMPILE = $(CC) $(CFLAGS) $(INCLUDES) -c

all: rocky
rocky: $(OBJFILES)
	$(CC) $(LDFLAGS) -o bin/$(EXECUTABLE) $(OBJFILES) $(LDFLAGS)


%.o: %.cpp
	$(COMPILE) -o $@ $<

.PHONY: clean
clean:
	rm -f $(SRCDIR)/*.o
