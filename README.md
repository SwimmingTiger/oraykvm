# 向日葵控控A2(OrayKVM)U盘救砖/USB恢复模式说明/免U盘刷机教程

控控A2是向日葵推出的IP KVM硬件盒子，通过模拟USB键鼠输入，并采集显卡HDMI输出，使受控电脑不需要安装远控软件就能被远程控制，甚至还能远程控制电脑进BIOS调设置，甚至远程重装系统，还是非常好用的。
https://sunlogin.oray.com/personal/kongkong2/

不过，这盒子还提供了SSH功能，而且还是root权限的。虽然会话chroot了，但要记住“chroot不是安全措施”。所以“chroot后的根目录看起来什么也没有”完全不能阻止我折腾。位于“SSH会话根目录之外”的`sunlogin_client`进程被我逆向了一番，顺便发现了它的隐藏功能（斗鱼直播推流）该怎么用（用法之后写）。

此外，通过直接访问`/dev/mtdblock2`，我完成了原厂固件的备份。然后用`file`命令确定这是`squashfs`固件，于是安装`squashfs-tools`进行解压、修改和重打包。

然后我就试着用`dd`命令把修改后的固件刷入控控，然而刷完就开不了机了。我猜也是如此，很多mtd设备用`dd if=/dev/new.img of=/dev/mtdblock2`这样的命令是无法正确写入的。

怎么办怎么办？之前看[控控开发者的领英](https://www.linkedin.com/in/%E7%9D%BF-%E8%94%A1-81945516b/)发现了这盒子具有`uboot中恢复系统等功能`：
> 此项目为知名远程控制软件向日葵的硬件化方案，用于解决传统软件版远程控制软件（如TeamViewer、向日葵等）必须依赖操作系统的问题，如传统软件必须进入主流操作系统后才能启动远控，且对系统权限有要求。可支持所有操作系统，且可在BIOS、工控机等环境下工作，满足了如运维人员等对远程安装操作系统等需求。
> 产品介绍页面请见：https://sunlogin.oray.com/zh_CN/kongkong2
> 
> 1. 负责产品硬件选型，使用海思hi3516a处理器，使用ADV7611 HDMI采集芯片并使用stm32f130c8t6单片机作为usb device模拟鼠标键盘设备；
> 2. 负责向日葵远程控制被控端软件编写，使用C++语言，基于嵌入式Linux，使用reactor机制进行异步IO编程（socket、串口等），并利用libjingle等框架，编写h264硬件压缩编码，音频采集与压缩模块、udp转发、p2p通讯模块，rpc相关模块等；
> 3. 负责根文件系统的搭建，使用builroot作为rootfs搭建系统，并将该产品所有的软件包集成至builroot中，方便代码的管理与维护；
> 4. 负责stm32f103c8t6单片机程序及主控芯片中的单片机通讯管理软件的编写，此单片机使用usb device模式，模拟为usb键盘鼠标设备，且主控芯片通过232串口与之通讯，并可升级、生产单片机中固件；
> 5. 负责部分嵌入式linux系统管理脚本、驱动等程序，负责uboot中恢复系统等功能，负责wifi、网口管理等；
> 6. 负责生产及测试相关任务。

但是我找遍了向日葵帮助中心，都没发现这个功能怎么用。不过按照玩路由器uboot的一贯经验，通常都是按住reset键再开机就进入恢复模式。我按了，果然不同寻常的红灯闪烁了起来。但是固件怎么放进去？网页？U盘？IP地址是什么？文件名是什么？

对uboot（/dev/mtdblock0）进行一番逆向之后，并没有什么收获，于是只好问客服。然后客服就发给我这个：

[USB刷入.rar（12.13 MB）](http://qiniu.img.hu60.cn/file-hash-rar-9623f23110f9165d1c7f15036672552e12724384.rar)

如链接的文件失效，请直接下载项目版本库中的同名文件。

# 控控A2 USB恢复模式说明

## 一、适用说明：
某些用户可能在升级过程断电或其他等原因造成的控控无法启动现象。
## 二、准备工作：
准备一个U盘（容量不限），若之前有分区，则尽量删除分区，只保留一个分区。将其分区格式化为FAT或FAT16文件系统(FAT32不支持，可使用diskgenius软件格式化)，拷贝kernel.bin文件和rootfs.bin文件至U盘当中。
## 三：具体操作：
将控控断电，插入刚刚准备好的U盘。按住reset键，再插入电源（顺序不能出错），会看到控控LED灯闪烁数次，此时松开reset键，将进入U盘恢复模式。控控会自动将U盘中的恢复文件恢复至控控中，此过程控控LED灯一直红灯常亮。持续2~3分钟后，控控将自动重启，并恢复成功。此时可拔下U盘，整个恢复过程完成。
## 四：可能遇到的问题及解决办法：
若有用户在上述操作中发现，控控LED灯闪烁数次后，没有常亮，或者常亮时间很短，又立即重启，说明控控未能识别U盘或其中文件。此时应检查U盘的文件系统是否为FAT格式，且U盘是否只有一个分区。

压缩包里面还附带了可供刷入的固件。

然后救砖很快就成功了，并且我还用同样的方法成功刷入了我修改的固件。果然修改不是问题，问题是不应该用`dd`刷入。

顺便吐槽一下，这么重要的救砖资料，为什么不放在官网上让我直接能找到呢？

--------------------------

# 免U盘刷机教程

U盘刷机虽然安全可靠，但来回插拔U盘以及按reset键，还是挺麻烦的，特别是在你测试自己修改的固件的时候，可能需要反复刷机验证然后调整。

所以，通过SSH直接刷固件是必须要实现的。我已经用亲身经历验证了`dd`命令不可行，那么该怎么刷呢？

解压客服提供的`USB刷入.rar`，再用7z解压里面的`rootfs.bin`，用记事本打开`etc/oray/sysupgradeEx`，这个是官方的固件升级脚本。里面有如下内容：
```
		if [ "$kernel_dl_ver" != "$kernel_cal_ver" ];then
			echo "CAUTION:upgrade kernel, do not disturb"	
			flash_erase /dev/mtd1 0x0 0x30
			flashcp $extract_path/uImage /dev/mtd1
		else
			echo "NO NEED TO UPGRADE KERNEL, skip..."	
		fi

		if [ "$rootfs_dl_ver" != "$rootfs_cal_ver" ];then
			echo "CAUTION:upgrade rootfs, do not disturb"	
			flash_erase /dev/mtd2 0x0 0xa0
			flashcp $extract_path/rootfs.squashfs /dev/mtd2
		else
			echo "NO NEED TO UPGRADE ROOTFS, skip..."	
		fi
```

这就是刷入内核和rootfs所需要的操作。不过，我们通过SSH端口44022登录的会话虽然是root权限，但是却chroot了，没有`flash_erase`和`flashcp`命令。所以，需要从解压的固件里面找到这两个二进制（在`usr/sbin`里），scp上去（可以用WinSCP，选SCP模式，或者用`scp`命令）。

下面演示在Linux终端中如何连接控控刷固件。假设我的控控IP是`192.168.1.22`，我电脑shell当前所在目录结构如下：
```
- kernel.bin
- rootfs.bin
+ squashfs-root (从rootfs.bin用`unsquashfs rootfs.bin`解压出来的，Windows可以用7zip解压)
    |- bin
    |- etc
    |- usr
    |- ...
```

控控的SSH监听在44022和44033端口。44022端口的用户名是`admin`，密码是`oray.com`，会话根目录被chroot了。44033的用户名是`root`，密码未知，会话根目录未被chroot。

所以我们只能连接44022进行刷机，并且需要从外部复制`flash_erase`和`flashcp`命令。只有更改固件修改了root密码后，才能连上44033端口。

## 备份原厂固件的方法

建议刷入前先对原厂固件进行备份，主要是备份串号等信息，防止刷机不当信息意外丢失。

```
# 登录SSH命令行shell
ssh -p44022 admin@192.168.1.22
# 密码是oray.com

# 以下是在控控SSH中执行的命令：
# dd命令虽然不能正确刷入固件，但是却可以正确备份固件
# 备份出来的固件可以通过下面提到的方法刷入

dd if=/dev/mtdblock0 of=/dev/mtd0.img
dd if=/dev/mtdblock1 of=/dev/mtd1.img
dd if=/dev/mtdblock2 of=/dev/mtd2.img
dd if=/dev/mtdblock3 of=/dev/mtd3.img
dd if=/dev/mtdblock4 of=/dev/mtd4.img
```

然后用`scp`或者`WinSCP`把这些`img`文件拷贝出来即可。
```
# 以下是在电脑终端中执行的命令
scp -P44022 admin@192.168.1.22:/dev/mtd0.img admin@192.168.1.22:/dev/mtd1.img admin@192.168.1.22:/dev/mtd2.img admin@192.168.1.22:/dev/mtd3.img admin@192.168.1.22:/dev/mtd4.img .
# 密码是oray.com
```

顺便解释一下每个文件（分区）的内容：
```
mtd0: uboot
mtd1: Linux内核
mtd2: squashfs 根文件系统
mtd3: jffs2 /config文件夹
mtd4: 数据区，内含串号，可能还有wifi校准数据等
```

所以一定要保护好你们的`mtd4`，否则刷坏了又被备份的话，以后可能会很麻烦。
此外，不要刷入`mtd0`或者`mtdblock0`，否则刷坏了就不能U盘救砖，只能返厂或者拆机上编程器。

## 刷机命令

```sh
# 把相关文件复制到`/dev`。这是chroot环境唯一的tmpfs内存盘。chroot环境不存在`/tmp`，需要自己挂载，不方便。
# 因为要刷根文件系统，所以固件肯定不能复制到根文件系统。更何况也没法复制过去，控控没有挂overlayfs，根文件系统是只读的。
# 控控有128M内存，平常占用很低，所以不用担心把内存塞满。但是不能塞太多，否则真的会满。
scp -P44022 rootfs.bin squashfs-root/usr/sbin/flash_erase squashfs-root/usr/sbin/flashcp admin@192.168.1.22:/dev/
# 密码是oray.com

# 登录SSH命令行shell
ssh -p44022 admin@192.168.1.22
# 密码是oray.com

接下来就是在控控的shell里面执行的命令了：
busybox chmod +x /dev/flash_erase /dev/flashcp

# 请像我这样一行输完命令，以免中途SSH断开发生意外。
# 这里我只刷了根文件系统，没有刷内核。如果你修改了内核，用同样的方式刷入`/dev/mtd1`即可。
# 【注意】，【不要动`/dev/mtd0`和`/dev/mtdblock0`】，否则砖了就只能返厂或者拆机上编程器了！！！
/dev/flash_erase /dev/mtd2 0x0 0xa0; /dev/flashcp /dev/rootfs.bin /dev/mtd2; echo done; busybox reboot -f

# 如果你看到了done，说明刷好了，控控已经在重启了。如果重启后卡在黄灯，恭喜，U盘救砖吧。祝你迅速进入白灯，然后系统顺利启动。
```

顺便提供两个原厂固件，里面的`rootfs.squashfs`重命名为`rootfs.bin`，`uImage`重命名为`kernel.bin`后可供U盘救砖。你也可以用我上面提到的方法免U盘刷入。再次警告，强烈不建议刷写【`/dev/mtd0`或`/dev/mtdblock0`】，也就是`u-boot.bin`。如果`uboot`刷坏了，救砖功能也就不能用了，只能返厂维修或者拆机用编程器重写uboot！

[kvm_upgrade_1.3.9_firmware.bz2（12.71 MB）](http://qiniu.img.hu60.cn/file-hash-bz2-35ceab1bc51eee0bec651623655e2bea13328328.bz2)

[kvm_upgrade_1.3.10_firmware.bz2（12.73 MB）](http://qiniu.img.hu60.cn/file-hash-bz2-24c6fafc3e9fef2a0bd3028de948f81513348043.bz2)

# 开启RTMP直播（比如斗鱼推流）的方法
1. 访问 `http://控控的IP:30080`，比如我这里是 `http://192.168.1.22:30080`
2. “请输入访问密码”，输入。
3. “是否开启RTMP直播”，选“是”。
4. RTMP地址，先输入推流地址，然后输入空格，再输入推流码。比如我这里是`rtmp://send3.douyu.com/live 58861..........?wsSecret=9.............&wsTime=5e7e5392&wsSeek=off&wm=0&tw=0&roirecognition=0`。
5. 点击“修改向日葵参数”。
6. 最后就是最坑的一点了：只有在通过向日葵软件连接到控控的时候，才会进行推流。关闭连接则推流停止。。。
7. 而且，推流的帧率只有20帧，挺卡的。
8. 没有声音。

# 在控控中通过chroot运行Debian/Raspbian（树莓派）系统

https://github.com/SwimmingTiger/oraykvm/releases/download/0.0.1/oraykvm-pi.tar.gz

链接：https://pan.baidu.com/s/13Bq4hVJTCeU-NCvT8zoTmw 
提取码：a875

使用方法：
1. U盘格式化为ext2文件系统。控控不支持ext3/4。
2. 把`oraykvm-pi.tar.gz`解压到U盘根目录。注意，为了保留文件权限和符号连接，必须在Linux里解压。
   如果要在控控内解压，请先在电脑里解开gzip压缩（Windows可以用7z，Linux用`gzip -d oraykvm-pi.tar.gz`）。
   在控控内解压gzip会非常非常非常非常非常非常非常非常慢，而仅解压tar则可以很快完成。
   tar解压命令参考：`busybox tar xf oraykvm-pi.tar`。在电脑上解压不需要`busybox`前缀。
3. U盘插入控控。
4. 登陆控控SSH，如`ssh admin@控控IP -p44022`
5. 挂载U盘，如`busybox mount /dev/sda1 /mnt`
6. 进入chroot系统：`/mnt/oraykvm-pi/chroot.sh`


# 控控固件修改/制作自定义固件

1. 启动Linux（或者WSL2），安装`squashfs-tools`软件包。<br>
   `sudo apt install squashfs-tools`
2. 解压固件。<br>
   `sudo unsquashfs rootfs.bin`
3. 修改固件，请自便。根文件系统在解压出来的`squashfs-root`文件夹里。
3. 打包固件。<br>
   `sudo mksquashfs squashfs-root new.bin -comp xz -b 131072`
4. 按前面提到的方法用U盘或者`flashcp`命令刷入`new.bin`

### 注意事项

不要在WSL的`/mnt`卷（Windows文件系统）内解压，否则文件权限可能不正确，特殊类型文件（比如设备文件）也会没有。

应该在WSL2或者原生Linux目录中解压。不推荐使用WSL1。不能解压到挂载的NTFS文件系统（比如`/media`或者`/mnt`下的目录）。

还有，需要使用`root`用户解压和打包（所以我给的命令前面都加了`sudo`）。

此外，刷入时不能使用`dd`命令。推荐使用U盘刷入。如果要免U盘，应该使用`flashcp`（看上面的免U盘刷机教程）。

# 模拟按键

用Shell命令就能控制控控A2的USB单片机进行按键模拟，参考这个脚本：[press.sh](press.sh)
