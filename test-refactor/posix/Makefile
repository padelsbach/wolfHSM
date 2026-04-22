## Makefile for wolfHSM test-refactor POSIX port
## Mirrors test/Makefile conventions with minimal sources

## Project name
BIN = wh_test_refactor

## Important directories
PROJECT_DIR      ?= .
REFACTOR_DIR     ?= $(PROJECT_DIR)/..
CONFIG_DIR       ?= $(REFACTOR_DIR)/../test/config
WOLFSSL_DIR      ?= ../../../wolfssl
WOLFHSM_DIR      ?= $(REFACTOR_DIR)/..
WOLFHSM_PORT_DIR ?= $(WOLFHSM_DIR)/port/posix
TEST_DIR         ?= $(WOLFHSM_DIR)/test

BUILD_DIR        ?= $(PROJECT_DIR)/Build

# Includes
INC =   -I$(PROJECT_DIR) \
        -I$(REFACTOR_DIR) \
        -I$(REFACTOR_DIR)/server \
        -I$(REFACTOR_DIR)/client-server \
        -I$(REFACTOR_DIR)/misc \
        -I$(CONFIG_DIR) \
        -I$(TEST_DIR) \
        -I$(WOLFSSL_DIR) \
        -I$(WOLFHSM_DIR) \
        -I$(WOLFHSM_PORT_DIR)

# POSIX requires C source be defined before any header
DEF += -D_POSIX_C_SOURCE=200809L

# Library configuration defines for user-supplied settings
DEF += -DWOLFSSL_USER_SETTINGS -DWOLFHSM_CFG

# Enable POSIX test features and server
DEF += -DWOLFHSM_CFG_TEST_POSIX
DEF += -DWOLFHSM_CFG_ENABLE_CLIENT
DEF += -DWOLFHSM_CFG_ENABLE_SERVER

# C standard
CSTD        ?= -std=c90

# Compiler flags
CFLAGS_EXTRA = -Werror -Wall -Wextra
CFLAGS_EXTRA += -ffunction-sections -fdata-sections
CFLAGS_EXTRA += -MMD -MP

ARCHFLAGS ?=
CFLAGS    ?= $(ARCHFLAGS) $(CSTD) $(CFLAGS_EXTRA)
LDFLAGS   ?= $(ARCHFLAGS)

# Dead-strip unused sections
OS_NAME := $(shell uname -s | tr A-Z a-z)
ifeq ($(OS_NAME),darwin)
    LDFLAGS += -Wl,-dead_strip
else
    LDFLAGS += -Wl,--gc-sections
endif

## Optional flags (same as test/Makefile)

ifeq ($(DEBUG),1)
    DBGFLAGS = -ggdb -g3
    CFLAGS += $(DBGFLAGS)
    LDFLAGS += $(DBGFLAGS)
    DEF += -DWOLFHSM_CFG_DEBUG
endif

ifeq ($(DEBUG_VERBOSE),1)
    DBGFLAGS = -ggdb -g3
    CFLAGS += $(DBGFLAGS)
    LDFLAGS += $(DBGFLAGS)
    DEF += -DWOLFHSM_CFG_DEBUG -DWOLFHSM_CFG_DEBUG_VERBOSE
endif

ifeq ($(ASAN),1)
    CFLAGS += -fsanitize=address
    LDFLAGS += -fsanitize=address
endif

## Source files

# wolfCrypt
SRC_C += $(wildcard $(WOLFSSL_DIR)/wolfcrypt/src/*.c)

# wolfSSL TLS (needed by cert manager APIs)
SRC_C += $(wildcard $(WOLFSSL_DIR)/src/*.c)

# wolfHSM library
SRC_C += $(wildcard $(WOLFHSM_DIR)/src/*.c)

# POSIX port (timestamps, flash file, etc.)
SRC_C += $(wildcard $(WOLFHSM_PORT_DIR)/*.c)

# Portable test-refactor sources (runner, groups)
SRC_C += $(wildcard $(REFACTOR_DIR)/*.c)

# Grouped test sources
SRC_C += $(wildcard $(REFACTOR_DIR)/server/*.c)
SRC_C += $(wildcard $(REFACTOR_DIR)/client-server/*.c)
SRC_C += $(wildcard $(REFACTOR_DIR)/misc/*.c)

# POSIX-specific sources (main, helpers)
SRC_C += $(wildcard $(PROJECT_DIR)/*.c)

# Legacy POSIX stress test pulled from test/; the .c file is
# self-gated on WOLFHSM_CFG_THREADSAFE et al. so it compiles to
# an empty TU when the gate isn't satisfied.
SRC_C += $(TEST_DIR)/wh_test_posix_threadsafe_stress.c


## Build rules

FILENAMES_C = $(notdir $(SRC_C))
OBJS_C = $(addprefix $(BUILD_DIR)/, $(FILENAMES_C:.c=.o))
vpath %.c $(dir $(SRC_C))

.PHONY: build_app clean run

build_app: $(BUILD_DIR) $(BUILD_DIR)/$(BIN).elf
	@echo Build complete.

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Workaround: pre-existing warnings in upstream files that
# -Werror promotes to errors.
$(BUILD_DIR)/internal.o: CFLAGS += -Wno-error=implicit-function-declaration
$(BUILD_DIR)/wh_client_crypto.o: CFLAGS += -Wno-error=sign-compare

$(BUILD_DIR)/%.o: %.c
	@echo "Compiling: $(notdir $<)"
	$(CC) $(CFLAGS) $(DEF) $(INC) -c -o $@ $<

-include $(OBJS_C:.o=.d)

$(BUILD_DIR)/$(BIN).elf: $(OBJS_C)
	@echo "Linking: $(notdir $@)"
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	@echo "Cleaning build files"
	@rm -rf $(BUILD_DIR)

run: build_app
	$(BUILD_DIR)/$(BIN).elf
