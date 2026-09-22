CC = gcc
PKG_CONFIG ?= pkg-config
PACKAGES = sdl3 sdl3-image
CFLAGS = -Wall -Wextra -std=c99
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags $(PACKAGES) 2>/dev/null)
LDLIBS += $(shell $(PKG_CONFIG) --libs $(PACKAGES) 2>/dev/null) -lm
SRC_DIR = src
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/image_processing.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = programa

.PHONY: all clean check-deps
all: $(EXECUTABLE)

check-deps:
	@$(PKG_CONFIG) --exists $(PACKAGES) || { echo "Instale SDL3 e SDL3_image (desenvolvimento) e configure PKG_CONFIG_PATH. Consulte README.md."; exit 1; }

$(EXECUTABLE): $(OBJECTS) | check-deps
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

%.o: %.c $(SRC_DIR)/image_processing.h | check-deps
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
