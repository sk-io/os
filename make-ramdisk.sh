#!/bin/bash -x

dd if=/dev/zero of=ramdisk.fat bs=8M count=1
LOOP=`losetup -f`
sudo losetup $LOOP ramdisk.fat
sudo mkfs.vfat $LOOP
sudo mount $LOOP mnt
sudo cp -r userspace/bin/* mnt/
sudo umount mnt
sudo losetup -d $LOOP
sync
sudo chown $SUDO_USER:$SUDO_USER ramdisk.fat
