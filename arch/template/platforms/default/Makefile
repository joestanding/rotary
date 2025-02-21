# --------------------------------------------------------------------------- #
# Makefile
# Architecture: REPLACE_ME_ARCHNAME
# Platform:     REPLACE_ME_PLATFORMNAME
# --------------------------------------------------------------------------- #

# Make automatic variables:
#	$@		target file
#	$<		first dependency
#	$^		all dependencies

# Variables inherited from parent Makefile:
# BUILD_DIR:    Top level directory for all build output
# OUTPUT_DIR:   Output directory for compiled object files
# KERNEL_DIR:   Architecture-agnostic kernel source
# ARCH_DIR:     Architecture-specific directory
# PLATFORM_DIR: Platform-specific directory

# --------------------------------------------------------------------------- #
# Configuration																  #
# --------------------------------------------------------------------------- #

.PHONY: all clean

# Specify the cross-compilation toolchain we need to use
CC_TOOLCHAIN := REPLACE_ME_TOOLCHAIN

# Build tool configuration
AFLAGS      := -f elf32

# --------------------------------------------------------------------------- #
# Directories																  #
# --------------------------------------------------------------------------- #

# Directory definitions go here

# --------------------------------------------------------------------------- #
# Source Files																  #
# --------------------------------------------------------------------------- #

$(OUTPUT_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT_DIR)/%.o: %.asm
	mkdir -p $(@D)
	$(ASM) $(AFLAGS) $< -o $@

$(OUTPUT_DIR)/%.o: %.S
	mkdir -p $(@D)
	$(CC) -c $< -o $@

# --------------------------------------------------------------------------- #
