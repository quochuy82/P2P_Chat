# Makefile for Modular P2P Chat Application
# Supports Linux, macOS, and Windows (MinGW)

# Program name
TARGET = p2p_chat

# Source files
SOURCES = main.c socket.c connection.c command.c signal.c
OBJECTS = $(SOURCES:.c=.o)

# Header files
HEADERS = common.h socket.h connection.h command.h signal.h

# Compiler
CC = gcc

# Common flags
CFLAGS = -Wall -Wextra -O2 -std=c99
DEBUG_FLAGS = -g -O0 -DDEBUG -fsanitize=address
LDFLAGS = 

# OS Detection
UNAME_S := $(shell uname -s 2>/dev/null || echo Windows)

# Platform-specific settings
ifeq ($(UNAME_S),Linux)
    PLATFORM = LINUX
    LDFLAGS += -pthread
    CFLAGS += -D_GNU_SOURCE
    RM = rm -f
    EXECUTABLE = $(TARGET)
    # Colors for Linux
    RED = \033[0;31m
    GREEN = \033[0;32m
    YELLOW = \033[0;33m
    BLUE = \033[0;34m
    NC = \033[0m
endif

ifeq ($(UNAME_S),Darwin)
    PLATFORM = MACOS
    LDFLAGS += -pthread
    RM = rm -f
    EXECUTABLE = $(TARGET)
    # Colors for macOS
    RED = \033[0;31m
    GREEN = \033[0;32m
    YELLOW = \033[0;33m
    BLUE = \033[0;34m
    NC = \033[0m
endif

# Windows detection
ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    PLATFORM = WINDOWS
    LDFLAGS += -lws2_32 -pthread
    CFLAGS += -D_WIN32
    RM = del /Q
    EXECUTABLE = $(TARGET).exe
endif

ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
    PLATFORM = WINDOWS
    LDFLAGS += -lws2_32 -pthread
    CFLAGS += -D_WIN32
    RM = del /Q
    EXECUTABLE = $(TARGET).exe
endif

# Default to Windows
ifeq ($(PLATFORM),)
    PLATFORM = WINDOWS
    LDFLAGS += -lws2_32 -pthread
    CFLAGS += -D_WIN32
    RM = del /Q
    EXECUTABLE = $(TARGET).exe
endif

# Phony targets
.PHONY: all clean debug release help run test install uninstall

# Default target
all: release

# Release build
release: CFLAGS += -DNDEBUG
release: $(EXECUTABLE)
	@echo "$(GREEN)====================================$(NC)"
	@echo "$(GREEN)Build successful!$(NC)"
	@echo "$(GREEN)Platform: $(PLATFORM)$(NC)"
	@echo "$(GREEN)Executable: $(EXECUTABLE)$(NC)"
	@echo "$(GREEN)====================================$(NC)"

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += -fsanitize=address
debug: $(EXECUTABLE)
	@echo "$(YELLOW)====================================$(NC)"
	@echo "$(YELLOW)Debug build successful!$(NC)"
	@echo "$(YELLOW)Platform: $(PLATFORM)$(NC)"
	@echo "$(YELLOW)Executable: $(EXECUTABLE)$(NC)"
	@echo "$(YELLOW)====================================$(NC)"

# Build executable
$(EXECUTABLE): $(OBJECTS)
	@echo "$(BLUE)Linking $(EXECUTABLE)...$(NC)"
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

# Compile source files
%.o: %.c $(HEADERS)
	@echo "$(BLUE)Compiling $<...$(NC)"
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
main.o: main.c common.h socket.h connection.h command.h signal.h
socket.o: socket.c socket.h common.h
connection.o: connection.c connection.h socket.h common.h
command.o: command.c command.h socket.h connection.h signal.h common.h
signal.o: signal.c signal.h socket.h common.h

# Clean build files
clean:
	@echo "$(YELLOW)Cleaning build files...$(NC)"
ifeq ($(PLATFORM),WINDOWS)
	-$(RM) *.o 2>NUL
	-$(RM) $(EXECUTABLE) 2>NUL
else
	$(RM) $(OBJECTS) $(EXECUTABLE)
	$(RM) -rf *.dSYM
endif
	@echo "$(GREEN)Clean complete!$(NC)"

# Run with default port
run: release
	@echo "$(GREEN)Starting P2P Chat on port 8080...$(NC)"
	./$(EXECUTABLE) 8080

# Run tests
test: debug
	@echo "$(BLUE)====================================$(NC)"
	@echo "$(BLUE)Running tests...$(NC)"
	@echo "$(BLUE)====================================$(NC)"
	@if [ -f $(EXECUTABLE) ]; then \
		echo "$(GREEN)✓ Build test passed$(NC)"; \
	else \
		echo "$(RED)✗ Build test failed$(NC)"; \
	fi
	@echo "$(BLUE)====================================$(NC)"

# Install (Unix-like systems)
install: release
ifeq ($(PLATFORM),WINDOWS)
	@echo "$(YELLOW)Manual installation required on Windows$(NC)"
	@echo "Copy $(EXECUTABLE) to desired location"
else
	@echo "$(BLUE)Installing to /usr/local/bin...$(NC)"
	@sudo cp $(EXECUTABLE) /usr/local/bin/
	@sudo chmod 755 /usr/local/bin/$(EXECUTABLE)
	@echo "$(GREEN)Installation complete!$(NC)"
endif

# Uninstall
uninstall:
ifeq ($(PLATFORM),WINDOWS)
	@echo "$(YELLOW)Manual uninstallation required on Windows$(NC)"
else
	@echo "$(BLUE)Uninstalling...$(NC)"
	@sudo rm -f /usr/local/bin/$(EXECUTABLE)
	@echo "$(GREEN)Uninstalled!$(NC)"
endif

# Help
help:
	@echo "$(BLUE)====================================$(NC)"
	@echo "$(BLUE)P2P Chat Application - Makefile Help$(NC)"
	@echo "$(BLUE)====================================$(NC)"
	@echo "Usage:"
	@echo "  make              - Build release version"
	@echo "  make debug        - Build debug version"
	@echo "  make clean        - Remove build files"
	@echo "  make run          - Run with port 8080"
	@echo "  make test         - Run basic tests"
	@echo "  make install      - Install to system (Unix)"
	@echo "  make uninstall    - Remove from system (Unix)"
	@echo "  make help         - Show this help"
	@echo ""
	@echo "Files:"
	@echo "  main.c       - Main entry point"
	@echo "  socket.c/h   - Socket operations"
	@echo "  connection.c/h - Connection management"
	@echo "  command.c/h  - Command processing"
	@echo "  signal.c/h   - Signal handling & utilities"
	@echo "  common.h     - Common definitions"
	@echo ""
	@echo "Platform: $(PLATFORM)"
	@echo "Compiler: $(CC)"
	@echo "$(BLUE)====================================$(NC)"