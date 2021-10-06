TARGET_NAME ?= TINYOS
TARGET_EXEC_HEX ?= $(TARGET_NAME).hex
TARGET_EXEC ?= $(TARGET_NAME).elf

MKDIR_P ?= mkdir -p

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

FLASHER_PATH = /opt/ti/MSPFlasher
COMPILER_BASE_DIR = /opt/ti/msp430-gcc

# compiler
CC = $(COMPILER_BASE_DIR)/bin/msp430-elf-gcc

# linker
LD = $(COMPILER_BASE_DIR)/bin/msp430-elf-gcc

SIZE=$(COMPILER_BASE_DIR)/bin/msp430-elf-size

OBJCOPY = $(COMPILER_BASE_DIR)/bin/msp430-elf-objcopy

#debug
DEBUG = -ggdb -gdwarf-2 -g3

#OPTIMIZATIONS
OPTIMIZE = -O0

WARN = -Wall

SRCS := $(shell find $(SRC_DIRS) -name \*.cpp -or -name \*.c -or -name \*.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) -I $(COMPILER_BASE_DIR)/include

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
CPPFLAGS = $(DEBUG) $(OPTIMIZE) $(WARN) $(INC_FLAGS) -MMD -MP -mmcu=msp430f449 \
			-DUSE_LTC2440
#-DTEST_SAMPLE_WAVEFORM

LDFLAGS = -mmcu=msp430f449  $(DEBUG) -L $(COMPILER_BASE_DIR)/include \
		  -T msp430f449.ld -lm
#-u _printf_float -u _scanf_float

$(BUILD_DIR)/$(TARGET_EXEC_HEX): $(BUILD_DIR)/$(TARGET_EXEC)
	@echo Creating HEX file
	@$(OBJCOPY) $^ -O ihex $@

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@echo Linking
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@$(SIZE) $@ -B

# assembly
$(BUILD_DIR)/%.s.o: %.s
	@echo AS $@
	@$(MKDIR_P) $(dir $@)
	@$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	@echo CC $@
	@$(MKDIR_P) $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	@echo CPP $@
	@$(MKDIR_P) $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

flash: $(BUILD_DIR)/$(TARGET_EXEC_HEX)
	$(FLASHER_PATH)/MSP430Flasher -i USB -w $^ -v -z [VCC=2.8,RESET]

flash-target: $(BUILD_DIR)/$(TARGET_EXEC_HEX)
	$(FLASHER_PATH)/MSP430Flasher -i USB -w $^ -v -z [VCC=2.8,RESET]

-include $(DEPS)
