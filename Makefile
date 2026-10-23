# fedii - Mastodon client for Nintendo 3DS
# Makefile

TARGET := fedii
BUILD_DIR := build
SOURCE_DIR := source
INCLUDE_DIR := include
GRAPHICS_DIR := graphics

# Compiler settings
CFLAGS := -g -O2 -Wall -Wextra
CFLAGS += -D_3DS -DARM11 -DARM9
CFLAGS += -I$(INCLUDE_DIR) -I$(PORTLIBS)/include

# Linker settings
LDFLAGS := -specs=3dsx.specs -g
LDFLAGS += -L$(PORTLIBS)/lib

# Libraries
LIBS := -lctru -lcitro3d -ljansson -lm

# Source files
SOURCES := \
    $(SOURCE_DIR)/main.c \
    $(SOURCE_DIR)/ui.c \
    $(SOURCE_DIR)/graphics.c \
    $(SOURCE_DIR)/mastodon.c \
    $(SOURCE_DIR)/network.c \
    $(SOURCE_DIR)/config.c

# Object files
OBJECTS := $(SOURCES:.c=.o)

# Targets
.PHONY: all clean cia

all: $(TARGET).3dsx

$(TARGET).3dsx: $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

cia: $(TARGET).cia

$(TARGET).cia: $(TARGET).3dsx
	@echo "Building CIA..."
	# CIA building would go here
	# For now, just copy the 3dsx
	cp $(TARGET).3dsx $(BUILD_DIR)/$(TARGET).3dsx

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(OBJECTS)
	rm -f $(TARGET).3dsx $(TARGET).cia
	rm -f $(TARGET).elf

# Dependency generation
%.d: %.c
	$(CC) -M $(CFLAGS) $< > $@.d 2>/dev/null

-include $(SOURCES:.c=.d)
