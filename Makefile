LIB_NAME = gemtext-extensions
LIB_DIR = ./lib
EXTERNAL_DIR ?= $(realpath ./)/external
SRC_DIR = ./src

INCLUDES = \
	$(GLOBAL_INCLUDES) \
	-I$(EXTERNAL_DIR)/gemtext/include \
	-I${SRC_DIR} \

LIBRARIES = \
	-L$(EXTERNAL_DIR)/gemtext/lib -lgemtext \

#
#
#
CC = gcc

CFLAGS = -Wall -Wextra -fPIC -g

SRC = \
	$(wildcard $(SRC_DIR)/*.c) \
	$(wildcard $(SRC_DIR)/smart-lists/*.c) \
	$(wildcard $(SRC_DIR)/thematic-break/*.c) \
	$(wildcard $(SRC_DIR)/markdown-link/*.c) \

OBJ_DIR = build
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

STATIC_LIB = $(LIB_DIR)/lib$(LIB_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(LIB_NAME).so

#
#
#

all: shared static

static: $(OBJS)
	mkdir -p $(LIB_DIR)
	ar rcs $(STATIC_LIB) $(OBJS)

shared: $(OBJS)
	mkdir -p $(LIB_DIR)
	$(CC) -shared -o $(SHARED_LIB) $(OBJS) $(LIBRARIES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBRARIES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(SHARED_LIB) $(STATIC_LIB)


.PHONY: all clean shared static
