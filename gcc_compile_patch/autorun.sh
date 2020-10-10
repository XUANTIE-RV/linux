# 在 common/ 目录下，执行 ./gcc_compile_patch/autorun.sh

echo "Check patch..."
git apply --check gcc_compile_patch/0001-Modify-to-compile-by-gcc.patch
echo -e "Check patch OK\n"

echo "Apply patch..."
git am gcc_compile_patch/0001-Modify-to-compile-by-gcc.patch
echo -e "Apply patch OK\n"

echo "Install toolchain..."
wget https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource/1836682/1600327810715/riscv64-linux-x86_64-20200820.tar.gz
mkdir -p host
tar zxf riscv64-linux-x86_64-20200820.tar.gz -C host
rm riscv64-linux-x86_64-20200820.tar.gz
echo -e "Install toolchain OK\n"

export LD_LIBRARY_PATH=host/lib
export ARCH=riscv
export CROSS_COMPILE=host/bin/riscv64-unknown-linux-gnu-

make Image -j48