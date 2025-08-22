# Makefile para Post-it Application

# Compilador y flags
CXX = g++
WINDRES = windres
CXXFLAGS = -DUNICODE -D_UNICODE -std=c++11 -Wall -Wextra -I include
LDFLAGS = -mwindows -luser32 -lgdi32 -lcomctl32 -lcomdlg32 -lshell32

# Directorios
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Archivos
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
RESOURCE = $(BUILD_DIR)/postit.res
TARGET = $(BUILD_DIR)/PostIt.exe

# Regla principal
all: $(TARGET)

# Crear directorio build si no existe
$(BUILD_DIR):
	@if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)

# Compilar el ejecutable
$(TARGET): $(OBJECTS) $(RESOURCE) | $(BUILD_DIR)
	$(CXX) $(OBJECTS) $(RESOURCE) -o $@ $(LDFLAGS)
	@echo Compilacion completada: $(TARGET)

# Compilar archivo de recursos
$(RESOURCE): $(SRC_DIR)/postit.rc | $(BUILD_DIR)
	cd $(SRC_DIR) && $(WINDRES) postit.rc -O coff -o ../$(RESOURCE)

# Compilar archivos objeto
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	@if exist $(BUILD_DIR)\*.o del $(BUILD_DIR)\*.o
	@if exist $(BUILD_DIR)\*.res del $(BUILD_DIR)\*.res
	@if exist $(TARGET) del $(TARGET)
	@echo Limpieza completada

# Ejecutar el programa
run: $(TARGET)
	@echo Ejecutando Post-it Application...
	@$(TARGET)

# Compilar con Visual Studio (alternativo)
vs-build: | $(BUILD_DIR)
	cl /EHsc /I $(INC_DIR) $(SRC_DIR)\*.cpp /Fe:$(TARGET) user32.lib gdi32.lib comctl32.lib comdlg32.lib

.PHONY: all clean run vs-build	
