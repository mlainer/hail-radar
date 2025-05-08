# Setze den Compiler und die Compiler-Flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall

# Definiere die Quellcode-Dateien
SRC = main.cpp
# Ziel-Objektdateien
OBJ = $(SRC:.cpp=.o)
# Ziel-Ausführbare Datei
TARGET = start_hail_radar

# Standardziel: Bauen der ausführbaren Datei
all: $(TARGET)

# Regel zum Erstellen der Ziel-Datei
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

# Regel zum Erstellen der Objektdateien
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ziel zum Löschen der erzeugten Dateien
clean:
	rm -f $(OBJ) $(TARGET) obj/*
