# 适用于控控A2的自定义内核

请注意：该内核仅适用于A2型号，不适用于A2-300型号。刷错型号需要[救砖](../A2-300/救砖文件)。

## 编译方法

请在 Ubuntu 20.04 内构建。亲测 Ubuntu 24.04 与该内核源代码不兼容。

```
# 安装编译器和依赖库
sudo apt update
sudo apt install git make gcc gcc-arm-linux-gnueabi gcc-arm-linux-gnueabihf bison flex u-boot-tools device-tree-compiler build-essential libncurses-dev

# 克隆源代码
git clone https://github.com/SwimmingTiger/oraykvm.git
cd oraykvm
git submodule update --init --depth=1 --progress

# 复制内核配置文件
cd custom-kernel
cp linux-3.4.y.config linux-3.4.y/.config

# 调整内核配置
cd linux-3.4.y
make menuconfig -j$(nproc) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

# 按需调整内核配置。
# 方向键上下移动选项，回车进入二级菜单，Esc返回上级菜单，空格切换状态（[ ]为禁用，[*]为启用，[M]为编译成ko模块）。
# 如果找不到配置项在哪里，可输入 / 进行配置项搜索，
# 然后根据 Depends on 打开依赖项（找不到依赖项在哪也可以用 / 搜索），
# 再根据 Location 找到配置并打开（如果不打开所有依赖项，配置就不会出现）。
# 修改好后按方向键左右键选中 <Exit> 退出并保存。

# 编译内核
make uImage -j$(nproc) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
file $PWD/arch/arm/boot/uImage

# 把 $PWD/arch/arm/boot/uImage 重命名为 kernel.bin 就可用于U盘刷入。

# 编译内核模块
make modules -j$(nproc) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
make modules_install -j$(nproc) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=$PWD/dist
find $PWD/dist -name '*.ko'

# 把 $PWD/dist/lib/modules/3.4.35/ 中你需要的 .ko 文件打包进固件的 /lib/moudles/3.4.35/ 目录即可。
# 注意：固件的 moudles 单词拼错了，u 在 d 前面，不是 modules。
```
