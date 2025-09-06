# Makefile for BMP Handler and WAV Handler Libraries (Cross-platform)

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET = temp/main.exe
    RM = del /Q
    RMDIR = rmdir /S /Q
    MKDIR = mkdir
else
    TARGET = temp/main
    RM = rm -f
    RMDIR = rm -rf
    MKDIR = mkdir -p
endif

# Directories
SRC_DIR = src
DRIVERS_DIR = $(SRC_DIR)/drivers
INCLUDE_DIR = $(SRC_DIR)/include
TEMP_DIR = temp

# Source files
BMP_SRC = $(DRIVERS_DIR)/bmp_handler.cpp
WAV_SRC = $(DRIVERS_DIR)/wav_handler.cpp
CANNY_SRC = $(INCLUDE_DIR)/canny.cpp
CONSTRUCTOR_SRC = $(INCLUDE_DIR)/constructor.cpp
PREVIEW_SRC = $(INCLUDE_DIR)/preview.cpp
PACK_SRC = $(INCLUDE_DIR)/pack.cpp
MAIN_SRC = $(SRC_DIR)/main.cpp

# Object files (all in temp directory)
BMP_OBJ = $(TEMP_DIR)/bmp_handler.o
WAV_OBJ = $(TEMP_DIR)/wav_handler.o
CANNY_OBJ = $(TEMP_DIR)/canny.o
CONSTRUCTOR_OBJ = $(TEMP_DIR)/constructor.o
PREVIEW_OBJ = $(TEMP_DIR)/preview.o
PACK_OBJ = $(TEMP_DIR)/pack.o
MAIN_OBJ = $(TEMP_DIR)/main.o

ALL_OBJS = $(BMP_OBJ) $(WAV_OBJ) $(CANNY_OBJ) $(CONSTRUCTOR_OBJ) $(PREVIEW_OBJ) $(PACK_OBJ) $(MAIN_OBJ)

# Libraries (in temp directory)
BMP_LIB = $(TEMP_DIR)/libbmp.a
WAV_LIB = $(TEMP_DIR)/libwav.a

# Default target
all: $(TARGET)

# Create temp directory if it doesn't exist
$(TEMP_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist $(TEMP_DIR) $(MKDIR) $(TEMP_DIR)
else
	@$(MKDIR) $(TEMP_DIR)
endif

# Compile main program
$(TARGET): $(ALL_OBJS) $(BMP_LIB) $(WAV_LIB) | $(TEMP_DIR)
ifeq ($(OS),Windows_NT)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJ) $(CANNY_OBJ) $(CONSTRUCTOR_OBJ) $(PREVIEW_OBJ) $(PACK_OBJ) -L$(TEMP_DIR) -lbmp -lwav -Wl,--stack,268435456
else
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(MAIN_OBJ) $(CANNY_OBJ) $(CONSTRUCTOR_OBJ) $(PREVIEW_OBJ) $(PACK_OBJ) -L$(TEMP_DIR) -lbmp -lwav -Wl,-z,stack-size=268435456
endif

# Compile BMP library
$(BMP_LIB): $(BMP_OBJ) | $(TEMP_DIR)
	ar rcs $(BMP_LIB) $(BMP_OBJ)

# Compile WAV library
$(WAV_LIB): $(WAV_OBJ) | $(TEMP_DIR)
	ar rcs $(WAV_LIB) $(WAV_OBJ)

# Compile object files
$(TEMP_DIR)/bmp_handler.o: $(BMP_SRC) | $(TEMP_DIR)
	$(CXX) $(CXXFLAGS) -c $(BMP_SRC) -o $(BMP_OBJ)

$(TEMP_DIR)/wav_handler.o: $(WAV_SRC) | $(TEMP_DIR)
	$(CXX) $(CXXFLAGS) -c $(WAV_SRC) -o $(WAV_OBJ)

$(TEMP_DIR)/canny.o: $(CANNY_SRC) | $(TEMP_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $(CANNY_SRC) -o $(CANNY_OBJ)

$(TEMP_DIR)/constructor.o: $(CONSTRUCTOR_SRC) | $(TEMP_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $(CONSTRUCTOR_SRC) -o $(CONSTRUCTOR_OBJ)

$(TEMP_DIR)/preview.o: $(PREVIEW_SRC) | $(TEMP_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $(PREVIEW_SRC) -o $(PREVIEW_OBJ)

$(TEMP_DIR)/pack.o: $(PACK_SRC) | $(TEMP_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $(PACK_SRC) -o $(PACK_OBJ)

$(TEMP_DIR)/main.o: $(MAIN_SRC) | $(TEMP_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $(MAIN_SRC) -o $(MAIN_OBJ)

# Clean target
clean:
ifeq ($(OS),Windows_NT)
	@if exist $(TEMP_DIR) $(RMDIR) $(TEMP_DIR) 2>nul || echo.
	@if exist *.bmp $(RM) *.bmp 2>nul || echo.
	@if exist *.wav $(RM) *.wav 2>nul || echo.
	@if exist *.txt $(RM) *.txt 2>nul || echo.
else
	@$(RMDIR) $(TEMP_DIR) 2>/dev/null || true
	@$(RM) *.bmp *.wav *.txt 2>/dev/null || true
endif

# Run main program
run: $(TARGET)
ifeq ($(OS),Windows_NT)
	$(TARGET)
else
	./$(TARGET)
endif

# Display help
help:
	@echo "Available targets:"
	@echo "  all          - Compile main program"
	@echo "  clean        - Clean generated files and temp directory"
	@echo "  run          - Compile and run main program"
	@echo "  help         - Display this help message"
	@echo ""
	@echo "Directory structure:"
	@echo "  src/            - Source code"
	@echo "  src/drivers/    - BMP and WAV handlers"
	@echo "  src/include/    - Modular components"
	@echo "  temp/           - Build artifacts (auto-created)"

.PHONY: all clean run help