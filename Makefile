CC = i686-elf-gcc
LD = i686-elf-gcc
CFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -m32 -c -Wall -Wextra -lgcc -O2
LDFLAGS = -g -std=gnu11 -nostdlib -ffreestanding -Tkernel.ld -lgcc -O2
ASFLAGS = -felf32

SOURCES_C = $(patsubst %.c, %.o, $(wildcard kernel/*.c) $(wildcard kernel/**/*.c))
SOURCES_ASM = $(patsubst %.asm, %.o, $(wildcard kernel/*.asm))

OBJ = $(SOURCES_ASM) $(SOURCES_C)

KERNEL = kernel.bin
IMAGE = os.iso
RAMDISK = ramdisk.fat

all: $(OBJ) link

link: $(OBJ)
	$(LD) -o $(KERNEL) $(OBJ) $(LDFLAGS)

kernel/%.o: kernel/%.c
	$(CC) $< -o $@ $(CFLAGS)

kernel/%.o: kernel/%.asm
	nasm $(ASFLAGS) $< -o $@

run: all image
	qemu-system-i386 -cdrom $(IMAGE) -serial stdio

drun: all
	qemu-system-i386 -s -S -cdrom $(IMAGE) -serial stdio

debug: all
	gdb --symbols=$(KERNEL) -ex 'target remote localhost:1234'

clean:
	make -C userspace clean
	rm -f kernel/*.o kernel/**/*.o $(KERNEL) $(IMAGE) $(RAMDISK)

image: all user
	cp $(KERNEL) image/boot
	cp $(RAMDISK) image/boot
	grub-mkrescue -o $(IMAGE) image

user:
	make -C userspace
