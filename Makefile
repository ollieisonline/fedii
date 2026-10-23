# fedii - Mastodon client for Nintendo 3DS
# Makefile

TARGET := fedii
BUILD_DIR := build
SOURCE_DIR := source
INCLUDE_DIR := include

# Toolchain
CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
STRIP := arm-none-eabi-strip

# Compiler settings
CFLAGS := -g -O2 -Wall -Wextra
CFLAGS += -D_3DS -DARM11
CFLAGS += -I$(INCLUDE_DIR)
CFLAGS += -I$(DEVKITARM)/arm-none-eabi/include
CFLAGS += -I$(LIBCTRU)/include
CFLAGS += -I$(PORTLIBS)/include
CFLAGS += -I$(PORTLIBS)/include/jansson

# Linker settings
LDFLAGS := -specs=3dsx.specs -g
LDFLAGS += -L$(LIBCTRU)/lib
LDFLAGS += -L$(PORTLIBS)/lib

# Libraries
LIBS := -lctru -lcitro3d -ljansson -lm -lz

# Source files
SOURCES := \
    $(SOURCE_DIR)/main.c \
    $(SOURCE_DIR)/ui.c \
    $(SOURCE_DIR)/graphics.c \
    $(SOURCE_DIR)/mastodon.c \
    $(SOURCE_DIR)/network.c \
    $(SOURCE_DIR)/config.c

# Object files
OBJECTS := $(patsubst $(SOURCE_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Targets
.PHONY: all clean cia

all: $(TARGET).3dsx

$(TARGET).3dsx: $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)
	@echo "Built $(TARGET).3dsx"

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

cia: $(TARGET).cia

$(TARGET).cia: $(TARGET).3dsx
	@echo "Building CIA..."
	@echo "Note: CIA building requires makerom tool"
	@echo "Copy $(TARGET).3dsx to SD card for testing"
	cp $< $(BUILD_DIR)/$@

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET).3dsx $(TARGET).cia $(TARGET).elf

# Dependency generation
%.d: %.c
	$(CC) -M $(CFLAGS) $< > $@.d 2>/dev/null

-include $(SOURCES:.c=.d)
