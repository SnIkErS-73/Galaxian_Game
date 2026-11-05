CC = gcc
CFLAGS = -Wall -Wextra -O2
SDL_CFLAGS = `sdl2-config --cflags`
SDL_LIBS = `sdl2-config --libs`

SRCS = main.c game.c player.c enemy.c bullet.c render.c
OBJS = $(SRCS:.c=.o)
TARGET = galaxian

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(SDL_LIBS) -lm

%.o: %.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run