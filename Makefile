# Makefile — Simulador de Tráfego Urbano em C
# Sistemas Operacionais — Concorrência, Sincronização e Deadlocks
#
# Alvos:
#   make        -> compila o binário em bin/simulador
#   make run    -> compila (se preciso) e executa a simulação
#   make clean  -> remove objetos e binário gerados

CC      := gcc
CFLAGS  := -Wall -Wextra -std=gnu11 -Iinclude -g
LDFLAGS := -pthread

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TARGET  := $(BIN_DIR)/simulador

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: all
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
