# --------------------------------------------------------------------------- #
# Broadcom BCM2837 Makefile
# ARM Cortex A53 (ARMv8)
# --------------------------------------------------------------------------- #

#	$@		target file
#	$<		first dependency
#	$^		all dependencies

# --------------------------------------------------------------------------- #
# Configuration																  #
# --------------------------------------------------------------------------- #

.PHONY: all clean

# The final kernel image for inclusion on the Pi SD card
FINAL_IMG := kernel8.img

# Specify the cross-compilation toolchain we need to use
CC_TOOLCHAIN := aarch64-rpi3-linux-gnu-

# Build tool configuration
ASM := $(CC_TOOLCHAIN)as

# --------------------------------------------------------------------------- #
# Directories																  #
# --------------------------------------------------------------------------- #

SDCARD_OUT_DIR := $(OUTPUT_DIR)/sdcard
SDCARD_IN_DIR  := $(PLATFORM_DIR)/sdcard

# --------------------------------------------------------------------------- #
# Source Files
# --------------------------------------------------------------------------- #

$(OUTPUT_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/%.o: %.asm
	mkdir -p $(@D)
	$(ASM) $< -o $@

$(OUTPUT_DIR)/%.o: %.S
	mkdir -p $(@D)
	$(CC) -c $< -o $@

# --------------------------------------------------------------------------- #
# Compiling Final Kernel Image												  #
# --------------------------------------------------------------------------- #

# Link together our final kernel image
$(OUTPUT_DIR)/kernel.bin: ${KERNEL_OBJ} ${ARCH_OBJ} ${TEST_OBJ}
	@echo "\n===== Creating final kernel image.. =====\n"
	$(LD) -Map $(OUTPUT_DIR)/linker.map -m aarch64elf -o $@ -T $(PLATFORM_DIR)/linker.ld $^
	nm $@ | sort | awk '{ print $$1" "$$3 }' > $(OUTPUT_DIR)/kernel-bochs.sym

# --------------------------------------------------------------------------- #
# Final Image 															      #
# --------------------------------------------------------------------------- #

$(SDCARD_OUT_DIR)/$(FINAL_IMG): $(OUTPUT_DIR)/kernel.bin
	@echo "\n===== Building final kernel image and SD card =====\n"
	# Link all compiled object files into one single ELF binary
	$(LD) -nostdlib $< -T $(PLATFORM_DIR)/linker.ld -o $(OUTPUT_DIR)/kernel8.elf
	# The bootloader doesn't work with ELF, so we copy it to a binary format
	$(OBJCOPY) -O binary $(OUTPUT_DIR)/kernel8.elf $(SDCARD_OUT_DIR)/$(FINAL_IMG)
	# Create a final SD card directory by copying the other required files
	cp -R $(SDCARD_IN_DIR)/* $(SDCARD_OUT_DIR)

# --------------------------------------------------------------------------- #

all: $(SDCARD_OUT_DIR)/$(FINAL_IMG)

# --------------------------------------------------------------------------- #
