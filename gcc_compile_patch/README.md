# README

本 patch 为使用 gcc 编译 android 内核时使用。编译完后可使用 `git reset hard` 恢复为原始工程。

【注意！！！】不要在应用本 patch 后提交代码。

## 执行方式

1. 应用 patch `gcc_compile_patch/0001-Modify-to-compile-by-gcc.patch`
   ```bash
   # 在 common/ 目录下
   git apply --check gcc_compile_patch/0001-Modify-to-compile-by-gcc.patch
   git am gcc_compile_patch/0001-Modify-to-compile-by-gcc.patch
   ```

2. 下载、安装工具链
   ```bash
   # 64 位工具链
   https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource/1836682/1600327810715/riscv64-linux-x86_64-20200820.tar.gz

   # 32 位工具链
   https://occ-oss-prod.oss-cn-hangzhou.aliyuncs.com/resource/1836682/1600328161531/riscv64-linux-i386-20200820.tar.gz

   将下载的工具链解压到 common/host 目录

   host
   ├── bin
   ├── include
   ├── lib
   ├── lib64
   ├── libexec
   ├── riscv64-unknown-linux-gnu
   ├── share
   └── sysroot

    # 配置环境变量
    export LD_LIBRARY_PATH=host/lib
    export ARCH=riscv
    export CROSS_COMPILE=host/bin/riscv64-unknown-linux-gnu-
   ```

3. 更新配置文件
   ```bash
   # 可以将 out/android-5.4-stable/common/.config 更新到 common/ 目录下，执行
   make menuconfig

   # 确认配置，并保存
   ```

4. 编译
   ```bash
   # 编译内核
   make Image -j12

   # 输出
   arch/riscv/boot/Image
   ```
