# Compilador e flags
CC = g++
CXXFLAGS = -std=c++17 -g -Wall
#CXXFLAGS = -std=c++17 -O3 -Wall

# Pastas
INCLUDE_FOLDER = ./include/
BIN_FOLDER = ./bin/
OBJ_FOLDER = ./obj/
SRC_FOLDER = ./src/

# Nome do executável
TARGET = main.exe

# Fontes e objetos
SRC = $(wildcard $(SRC_FOLDER)*.cpp) $(wildcard $(SRC_FOLDER)*.c)
OBJ = $(patsubst $(SRC_FOLDER)%.cpp, $(OBJ_FOLDER)%.o, $(SRC))
OBJ := $(patsubst $(SRC_FOLDER)%.c, $(OBJ_FOLDER)%.o, $(OBJ))

# Bibliotecas
LIBS = -lglfw3 -lopengl32 -lgdi32

# Regra para compilar cada .cpp
$(OBJ_FOLDER)%.o: $(SRC_FOLDER)%.cpp
	$(CC) $(CXXFLAGS) -I$(INCLUDE_FOLDER) -c $< -o $@

# Regra para compilar cada .c
$(OBJ_FOLDER)%.o: $(SRC_FOLDER)%.c
	$(CC) $(CXXFLAGS) -I$(INCLUDE_FOLDER) -c $< -o $@

# Regra principal
all: $(OBJ)
	$(CC) $(CXXFLAGS) -o $(BIN_FOLDER)$(TARGET) $(OBJ) $(LIBS)

# Limpeza
clean:
	@rm -rf $(OBJ_FOLDER)* $(BIN_FOLDER)*

