CC = clang
LD = clang
CFLAGS = -g -nostdlib -ffreestanding -m32 -fno-builtin -no-pie -Wall -Wextra -Isgfx
LDFLAGS = -g -nostdlib -ffreestanding -m32 -fno-builtin -no-pie -Tkernel.ld -Lsgfx -lsgfx -lgcc
ASFLAGS = -felf32

SOURCES_C = $(patsubst %.c, %.o, $(wildcard kernel/*.c) $(wildcard kernel/**/*.c))
SOURCES_ASM = $(patsubst %.asm, %.o, $(wildcard kernel/*.asm))

OBJ = $(SOURCES_ASM) $(SOURCES_C)

KERNEL = kernel.bin
IMAGE = os.iso
RAMDISK = ramdisk.fat

all: $(IMAGE)

$(IMAGE): $(KERNEL) $(RAMDISK)
	cp $(KERNEL) image/boot
	cp $(RAMDISK) image/boot
	grub-mkrescue -o $(IMAGE) image

$(KERNEL): $(OBJ) libsgfx
	$(LD) -o $(KERNEL) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

%.o: %.asm
	nasm $(ASFLAGS) $< -o $@

$(RAMDISK): user
	dd if=/dev/zero of=$(RAMDISK) bs=8M count=1
	mformat -i $(RAMDISK) ::
	mcopy -i $(RAMDISK) userspace/bin/* ::

user: libsgfx
	make -C userspace

libsgfx:
	make -C sgfx

run: $(IMAGE)
	qemu-system-i386 -cdrom $(IMAGE) -serial stdio -accel kvm

drun: $(IMAGE)
	qemu-system-i386 -s -S -cdrom $(IMAGE) -serial stdio

debug:
	gdb --symbols=$(KERNEL) -ex 'target remote localhost:1234'

clean:
	make -C userspace clean
	make -C sgfx clean
	rm -f kernel/*.o kernel/**/*.o $(KERNEL) $(IMAGE) $(RAMDISK)
