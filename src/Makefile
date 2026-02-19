# Compilateur et flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -O3 # -pg 

# Liste des fichiers source et des objets
SRCS = Datas.cpp greedy.cpp main2.cpp  \
	GradientDescent.cpp SimulatedAnnealing.cpp LocalSearchMethods.cpp \
	Common.cpp Conductor.cpp FamilliesTSP.cpp
OBJS = $(SRCS:.cpp=.o)

# Nom de l'exécutable
TARGET = prog

# Règle par défaut
all: $(TARGET)

# Lien des objets pour créer l'exécutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compilation des fichiers .cpp en .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Nettoyage des fichiers objets et de l'exécutable
clean:
	rm -f $(OBJS) $(TARGET)

# Règle pratique pour recompiler complètement
rebuild: clean all
