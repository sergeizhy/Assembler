CC		= gcc
CFLAGS		= -g -Wall -ansi -pedantic
SRC_DIR 	= src
OUT_DIR 	= out
OBJ_DIR 	= $(OUT_DIR)/obj
BIN_DIR 	= $(OUT_DIR)/bin
PROGRAM 	= asm
TARGET		= $(BIN_DIR)/$(PROGRAM)


HEADERS 	= $(wildcard $(SRC_DIR)/headers/*.h)
SOURCES 	= $(wildcard $(SRC_DIR)/*.c)
OBJECTS		= $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean

all: $(OUT_DIR) $(TARGET)

$(OUT_DIR): $(BIN_DIR) $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OUT_DIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

clean:
	rm -rf $(OUT_DIR)