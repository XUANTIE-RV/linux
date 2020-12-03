#!/bin/bash
# $1 option:
#       kernel:
#               build-gcc.sh gki_defconfig
#               build-gcc.sh Image
#       dtb:
#               build-gcc.sh dtbs
export PATH=$PATH:/home/majun/ice-910/sdk-toolchain

make  $1  -j16 ARCH=riscv  CROSS_COMPILE=/home/majun/ice-910/sdk-toolchain/riscv64-unknown-linux-gnu-

if [ "$1" == 'Image' ]; then
    echo "start to generate the uImage"
    ../../tools/mkimage  -A riscv -O linux -T kernel -C none -a 0x00200000 -e 0x00200000 -n Linux -d ./arch/riscv/boot/Image ./uImage
fi
