# Compilador e Flags
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Iinclude

# Nome do executável 
TARGET = trabalho

# Diretórios
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Localiza todos os arquivos .c dentro de src/ e a main.c
SRCS = $(wildcard $(SRC_DIR)/*.c) main.c
OBJS = $(patsqatsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))

# Regra principal
all: $(OBJ_DIR) $(TARGET)

# Cria o executável compilando os objetos
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

# Regra para compilar os arquivos de src/ em obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar a main.c
$(OBJ_DIR)/main.o: main.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Cria o diretório de objetos se não existir
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Limpeza
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean