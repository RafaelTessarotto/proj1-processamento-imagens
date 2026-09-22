CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lSDL3 -lSDL3_image -lSDL3_ttf -lm
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/image_processing.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = programa

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(EXECUTABLE)