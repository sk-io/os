# Untitled 32bit Graphical OS

![Screenshot](res/screenshot.png)

## Features/Limitations

* Higher half multitasking kernel
* Essential stuff like paging, interrupts, PS2 mouse/keyboard support
* FAT filesystem via FatFS
* Userspace
* Framebuffer graphics (using shared memory)
* Event system (using shared memory)
* Shared libraries
* A single userspace OS API a la winapi

Currently relies on multiboot/grub to provide a ramdisk and a framebuffer.

## Architecture/Motivation

This is a completely monolithic OS with a single userspace API for everything (task management, file I/O, window management, etc...). The kernel is not unix based.
All an application needs to do is to include "os.h" and link against api.so.

I try to keep things as simple and understandable as possible whilst still being speedy. There are tons of security holes, that's not a priority right now.

Current goal is to port Doom and some other stuff. :)

## Building

Requires clang, 32bit libgcc (gcc-multilib on debian), NASM, mtools and xorriso.

To build:
```
make
```
To build and run (using qemu):
```
make run
```
