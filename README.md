# Rotary
Rotary is a custom operating system and kernel, developed as a learning exercise and fun personal project. It's written primarily in C and Assembly, and currently features support for 32-bit x86 platforms (with capability to support more in future).

Rotary currently includes:
- An **assembly-based bootloader** that hands control to the C kernel after initialization.
- A **[buddy allocator](https://en.wikipedia.org/wiki/Buddy_memory_allocation)** for efficient physical memory management.
- A **[slab allocator](https://en.wikipedia.org/wiki/Slab_allocation)** for general-purpose, virtually mapped memory allocation.
- **Pre-emptive multitasking** with a round-robin scheduler.
- **User-mode tasks**, each with its own virtual address space.
- A **unit testing framework** for validating kernel components.

### **Work in Progress**
- A **Virtual File System (VFS)** inspired by Linux's design.
- Support for **block devices** and the **ext2 filesystem**

Blog posts outlining my development process in the above features can be found on [my website](https://joestanding.co.uk).

Much of Rotary's implementation is influenced by the Linux kernel, which I studied as a reference when researching each required component and subsystem.

Although the project currently only supports 32-bit x86, architecture-specific code is separated into its own directories and support for new architectures can be added by implementing the necessary functionality within architecture-specific code.

### Documentation

High-level documentation for the project can be found at [Rotary Docs](https://rotary.joestanding.co.uk), which provides information on the various subsystems and their implementations. This documentation is intended to give a simple overview of the project's architecture.

### Directory Structure

The directory structure is relatively simple, with the following layout:
- `/kernel` - Platform-agnostic kernel components (e.g. task implementations)
- `/include` - Platform-agnostic header files
- `/arch/<arch>/kernel` - Platform-specific kernel code (e.g. Intel 8259 PIC code)
- `/arch/<arch>/include/arch` - Platform-specific header files
- `/build/iso` - Files used for compiling the final image (e.g. GRUB configuration)
- `/test` - Test suites that can be booted from the GRUB boot menu
- `/user` - User-space application code

### Installation Requirements

To install the requirements on an apt-based system such as Ubuntu, the following command can be used:

`$ sudo apt install make nasm gcc xorriso mtools bochs bochs-x`

This will install the tools necessary for assembly, compilation, and emulation. Rotary currently uses Bochs as the preferred x86 emulator, however QEMU can also be used.

### Running in an Emulator

To run Rotary using the preferred emulator Bochs, you can do a clean build and run with the following:

`$ make clean && make && make run-bochs`

Bochs should then appear with a debugging window and the emulated video output.

### Output

Currently, debugging information is logged to the COM2 serial port - by default, Bochs will save this output into a `com2.txt` text file in the root directory. Tailing this file during execution will provide debugging output, such as the example below:
```
[arch/x86/kernel/cpu.c] cpu_init_gdt() - Initialising GDT..
[arch/x86/kernel/interrupts.c] IDT addr: 0xc01240a0, descriptor 0xc01248a0
[arch/x86/kernel/pic8259.c] Sending ICW1_INIT to PIC1
[arch/x86/kernel/pic8259.c] Setting PIC offsets [PIC1: 32 | PIC2: 40]
```
