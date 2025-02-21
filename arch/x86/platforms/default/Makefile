# --------------------------------------------------------------------------- #
# x86 Default Makefile										  				  #
# --------------------------------------------------------------------------- #

#	$@		target file
#	$<		first dependency
#	$^		all dependencies

# --------------------------------------------------------------------------- #
# Configuration										  						  #
# --------------------------------------------------------------------------- #

.PHONY: all clean run-qemu run-bochs

# Build tools
QEMUX86		:= qemu-system-i386
BOCHS		:= bochs
ASM			:= nasm
AFLAGS      := -f elf32

# Filenames
FINAL_ISO_FILENAME := final-image.iso

# --------------------------------------------------------------------------- #
# Directories																  #
# --------------------------------------------------------------------------- #

ISO_UNPACKED_DIR := $(BUILD_DIR)/iso
EMU_DIR := $(ARCH_DIR)/platforms/$(PLATFORM)/emulation

# --------------------------------------------------------------------------- #
# Final ISO and Kernel Image 												  #
# --------------------------------------------------------------------------- #

# Create the final ISO image which will use GRUB as a bootloader
$(OUTPUT_DIR)/$(FINAL_ISO_FILENAME): $(OUTPUT_DIR)/kernel.bin
	@echo "\n===== Building final ISO =====\n"
	mkdir -p $(ISO_UNPACKED_DIR)
	cat $^ > $@
	cp $(OUTPUT_DIR)/kernel.bin $(ISO_UNPACKED_DIR)/boot/os.bin
	grub-mkrescue -o $(OUTPUT_DIR)/$(FINAL_ISO_FILENAME) $(ISO_UNPACKED_DIR)

# Link together our final kernel image
$(OUTPUT_DIR)/kernel.bin: ${KERNEL_OBJ} ${ARCH_OBJ} ${TEST_OBJ}
	@echo "\n===== Linking final kernel image (kernel.bin).. =====\n"
	$(LD) -Map $(OUTPUT_DIR)/linker.map -m elf_i386 -o $@ -T $(PLATFORM_DIR)/linker.ld $^
	nm $@ | sort | awk '{ print $$1" "$$3 }' > $(OUTPUT_DIR)/kernel-bochs.sym

# --------------------------------------------------------------------------- #
# Source Files
# --------------------------------------------------------------------------- #

$(OUTPUT_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -m32 -c $< -o $@

$(OUTPUT_DIR)/%.o: %.asm
	mkdir -p $(@D)
	$(ASM) $(AFLAGS) $< -o $@

# --------------------------------------------------------------------------- #
# Emulation									  								  #
# --------------------------------------------------------------------------- #

run-qemu:
	$(QEMUX86) -cdrom $(OUTPUT_DIR)/$(FINAL_ISO_FILENAME) -monitor stdio -d cpu_reset

run-bochs:
	$(BOCHS) -f $(EMU_DIR)/bochs.config -q -rc $(EMU_DIR)/bochs.rc

# --------------------------------------------------------------------------- #

all: $(OUTPUT_DIR)/$(FINAL_ISO_FILENAME)

# --------------------------------------------------------------------------- #
