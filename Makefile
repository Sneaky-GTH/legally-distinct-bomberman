# Run "make help" to see usage instructions.

# Directories
SRC_DIR    := src
BUILD_DIR  := build
OBJ_DIR    := $(BUILD_DIR)/obj

CLIENT_SRC_DIR := $(SRC_DIR)/client
SERVER_SRC_DIR := $(SRC_DIR)/server
LIB_SRC_DIR    := $(SRC_DIR)/lib

# Output names
CLIENT_BIN := $(BUILD_DIR)/client
SERVER_BIN := $(BUILD_DIR)/server
SHARED_LIB := $(BUILD_DIR)/libbomberman.so

# Toolchain
CC := gcc
LD := gcc

## Flags

# Common compiler flags (always applied)
COMMON_CFLAGS := -Wall -Wextra -fPIC -I$(SRC_DIR) -I$(LIB_SRC_DIR) -D_GNU_SOURCE

# Usage: "make release=1 ..."
ifeq ($(release),1)
  MODE := release
endif

ifeq ($(MODE),release)
# Optimization flags for release mode
# TODO: add more flags like -flto, -fomit-frame-pointer, etc. after testing
  OPT_CFLAGS := -O2 -DNDEBUG -march=native
else
  OPT_CFLAGS := -g -O0 -DDEBUG
endif

# For the future
CLIENT_CFLAGS  := -lGL -lGLU -lglut
SERVER_CFLAGS  :=
LIB_CFLAGS     :=

LIB_LDFLAGS    := -shared

# Per-binary linker flags
CLIENT_LDFLAGS := -L$(BUILD_DIR) -Wl,-rpath,'$$ORIGIN/../$(BUILD_DIR)' -lGL -lGLU -lglut
SERVER_LDFLAGS := -L$(BUILD_DIR) -Wl,-rpath,'$$ORIGIN/../$(BUILD_DIR)'

# Libraries to link against (will need -lpthread, -lm, etc.)
CLIENT_LIBS    := -lbomberman
SERVER_LIBS    := -lbomberman

# Assembled flag sets (!!do not edit, modify the variables above)
CFLAGS        := $(COMMON_CFLAGS) $(OPT_CFLAGS)
CLIENT_ALL_CF := $(CFLAGS) $(CLIENT_CFLAGS)
SERVER_ALL_CF := $(CFLAGS) $(SERVER_CFLAGS)
LIB_ALL_CF    := $(CFLAGS) $(LIB_CFLAGS)

# Source and object files
CLIENT_SRCS := $(shell find $(CLIENT_SRC_DIR) -name '*.c')
SERVER_SRCS := $(shell find $(SERVER_SRC_DIR) -name '*.c')
LIB_SRCS    := $(shell find $(LIB_SRC_DIR) -name '*.c')
FORMAT_SRCS := $(shell find $(SRC_DIR) -type f \( -name '*.c' -o -name '*.h' \))

CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRCS))
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRCS))
LIB_OBJS    := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SRCS))

# Okay begin the actual Makefile here
.PHONY: all client server lib run_client run_server clean info format format-check

all: client server

client: $(CLIENT_BIN)
server: $(SERVER_BIN)
lib:    $(SHARED_LIB)

# Link Binaries
$(CLIENT_BIN): $(CLIENT_OBJS) $(SHARED_LIB) | $(BUILD_DIR)
	$(LD) $(CLIENT_OBJS) $(CLIENT_LDFLAGS) $(CLIENT_LIBS) -o $@
	@echo "[LINK] $@"

$(SERVER_BIN): $(SERVER_OBJS) $(SHARED_LIB) | $(BUILD_DIR)
	$(LD) $(SERVER_OBJS) $(SERVER_LDFLAGS) $(SERVER_LIBS) -o $@
	@echo "[LINK] $@"

# Link Shared Library
$(SHARED_LIB): $(LIB_OBJS) | $(BUILD_DIR)
	$(LD) $(LIB_LDFLAGS) $^ -o $@
	@echo "[LIB]  $@"

# Compile files
$(OBJ_DIR)/client/%.o: $(CLIENT_SRC_DIR)/%.c | $(OBJ_DIR)/client
	@mkdir -p $(dir $@)
	$(CC) $(CLIENT_ALL_CF) -c $< -o $@
	@echo "[CC]   $<"

$(OBJ_DIR)/server/%.o: $(SERVER_SRC_DIR)/%.c | $(OBJ_DIR)/server
	@mkdir -p $(dir $@)
	$(CC) $(SERVER_ALL_CF) -c $< -o $@
	@echo "[CC]   $<"

$(OBJ_DIR)/lib/%.o: $(LIB_SRC_DIR)/%.c | $(OBJ_DIR)/lib
	@mkdir -p $(dir $@)
	$(CC) $(LIB_ALL_CF) -c $< -o $@
	@echo "[CC]   $<"

# Create necessary directories
$(BUILD_DIR):
	@mkdir -p $@

$(OBJ_DIR)/client:
	@mkdir -p $@

$(OBJ_DIR)/server:
	@mkdir -p $@

$(OBJ_DIR)/lib:
	@mkdir -p $@

# Extra utility recipes
run_client: $(CLIENT_BIN)
	./$(CLIENT_BIN) $(ARGS)

run_server: $(SERVER_BIN)
	./$(SERVER_BIN) $(ARGS)

clean:
	rm -rf $(BUILD_DIR)
	@echo "Removed $(BUILD_DIR)/"

format:
	clang-format -i $(FORMAT_SRCS)
	@echo "Formatted source files."

format-check:
	@status=0; \
	for file in $(FORMAT_SRCS); do \
		tmp=$$(mktemp); \
		clang-format "$$file" > "$$tmp"; \
		if ! cmp -s "$$file" "$$tmp"; then \
			echo "[FORMAT] $$file needs formatting"; \
			status=1; \
		fi; \
		rm -f "$$tmp"; \
	done; \
	exit $$status

info:
	@echo "Mode          : $(if $(filter release,$(MODE)),release,debug)"
	@echo "CC            : $(CC)"
	@echo "LD            : $(LD)"
	@echo "CFLAGS        : $(CFLAGS)"
	@echo "CLIENT_CFLAGS : $(CLIENT_ALL_CF)"
	@echo "SERVER_CFLAGS : $(SERVER_ALL_CF)"
	@echo "LIB_CFLAGS    : $(LIB_ALL_CF)"
	@echo "CLIENT_LDFLAGS: $(CLIENT_LDFLAGS) $(CLIENT_LIBS)"
	@echo "SERVER_LDFLAGS: $(SERVER_LDFLAGS) $(SERVER_LIBS)"
	@echo "Client sources: $(CLIENT_SRCS)"
	@echo "Server sources: $(SERVER_SRCS)"
	@echo "Lib    sources: $(LIB_SRCS)"

help:
	@echo "Usage: make [target] [release=1] [ARGS=\"...\"]"
	@echo "Targets:"
	@echo "  all (default) - Build both client and server"
	@echo "  client        - Build client binary"
	@echo "  server        - Build server binary"
	@echo "  lib           - Build shared library"
	@echo "  format        - Format all C and header files with clang-format"
	@echo "  format-check  - Check whether all C and header files are clang-format clean"
	@echo "  run_client    - Run the client (requires client binary)"
	@echo "  run_server    - Run the server (requires server binary)"
	@echo "  clean         - Remove all build artifacts"
	@echo "  info          - Print build configuration and source files"
	@echo "Options:"
	@echo "  release=1     - Build in release mode (enables optimizations and disables debug info)"
	@echo "  ARGS=\"...\"    - Arguments to pass when running client/server with run_client/run_server"
