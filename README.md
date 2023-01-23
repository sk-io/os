# Untitled 32bit Graphical OS

![Screenshot](res/screenshot.png)

## Features

* Essential stuff like paging, interrupts, PS2 mouse/keyboard support
* Higher half kernel
* FAT filesystem via FatFS and a grub provided ramdisk
* Basic loading of ELF binaries
* Shared memory
* Framebuffer graphics using shared memory

## Architecture/Motivation

This is a completely monolithic OS with a single userspace API for everything (task management, file I/O, window management, etc...).
All an application needs to do is to include "os.h" and statically link against libos.a (Shared library in the future).

Current goal is to port Doom and some other stuff.

## Building

Requires i686-elf-gcc and NASM.

To build and run (requires sudo access):
```
make
./make-ramdisk.sh
make run
```


