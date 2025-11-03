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
BOIDS_TARGET = boids.exe

# Fontes comuns (excluindo os mains)
COMMON_SRC = $(filter-out $(SRC_FOLDER)main.cpp $(SRC_FOLDER)boids_main.cpp, $(wildcard $(SRC_FOLDER)*.cpp)) $(wildcard $(SRC_FOLDER)*.c)
COMMON_OBJ = $(patsubst $(SRC_FOLDER)%.cpp, $(OBJ_FOLDER)%.o, $(COMMON_SRC))
COMMON_OBJ := $(patsubst $(SRC_FOLDER)%.c, $(OBJ_FOLDER)%.o, $(COMMON_OBJ))

# Objetos específicos
MAIN_OBJ = $(OBJ_FOLDER)main.o
BOIDS_MAIN_OBJ = $(OBJ_FOLDER)boids_main.o

# Bibliotecas
LIBS = -lglfw3 -lopengl32 -lgdi32

# Regra para compilar cada .cpp
$(OBJ_FOLDER)%.o: $(SRC_FOLDER)%.cpp
	$(CC) $(CXXFLAGS) -I$(INCLUDE_FOLDER) -c $< -o $@

# Regra para compilar cada .c
$(OBJ_FOLDER)%.o: $(SRC_FOLDER)%.c
	$(CC) $(CXXFLAGS) -I$(INCLUDE_FOLDER) -c $< -o $@

# Regra principal - compila ambos os executáveis
all: $(BIN_FOLDER)$(TARGET) $(BIN_FOLDER)$(BOIDS_TARGET)

# Compila o main original
$(BIN_FOLDER)$(TARGET): $(COMMON_OBJ) $(MAIN_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compila o sistema de boids
$(BIN_FOLDER)$(BOIDS_TARGET): $(COMMON_OBJ) $(BOIDS_MAIN_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compila apenas boids
boids: $(BIN_FOLDER)$(BOIDS_TARGET)

# Limpeza
clean:
	@rm -rf $(OBJ_FOLDER)* $(BIN_FOLDER)*

