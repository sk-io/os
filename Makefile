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

all: $(IMAGE)

$(KERNEL): $(OBJ)
	$(LD) -o $(KERNEL) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $< -o $@ $(CFLAGS)

%.o: %.asm
	nasm $(ASFLAGS) $< -o $@

run: $(IMAGE)
	qemu-system-i386 -cdrom $(IMAGE) -serial stdio

drun: $(IMAGE)
	qemu-system-i386 -s -S -cdrom $(IMAGE) -serial stdio

debug:
	gdb --symbols=$(KERNEL) -ex 'target remote localhost:1234'

clean:
	make -C userspace clean
	rm -f kernel/*.o kernel/**/*.o $(KERNEL) $(IMAGE) $(RAMDISK)

$(IMAGE): user $(KERNEL)
	cp $(KERNEL) image/boot
	cp $(RAMDISK) image/boot
	grub-mkrescue -o $(IMAGE) image

user:
	make -C userspace
