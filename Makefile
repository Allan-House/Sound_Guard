# Nome do executável
TARGET = build/main

# Diretórios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Fonte e objetos
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))

# Compilador e flags
CC ?= gcc
CFLAGS = -Wall -O2 -I$(INC_DIR)
LDFLAGS = -lpigpio -lrt -lpthread

# Regra principal
all: $(BUILD_DIR) $(TARGET)

# Cria o diretório build se não existir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Linka os objetos para gerar o executável
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compila os arquivos .c em .o no build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -rf $(BUILD_DIR)
