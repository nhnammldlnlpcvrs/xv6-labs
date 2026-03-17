#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
tree(char *path, int depth)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    // Mở thư mục
    if((fd = open(path, 0)) < 0){
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    // Lấy thông tin file
    if(fstat(fd, &st) < 0){
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // Nếu không phải thư mục thì in ra luôn
    if(st.type != T_DIR){
        for(int i = 0; i < depth; i++)
            printf("  ");
        printf("%s\n", path);
        close(fd);
        return;
    }

    // In tên thư mục
    for(int i = 0; i < depth; i++)
        printf("  ");
    printf("%s/\n", path);

    // Duyệt từng entry trong thư mục
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;

        // Bỏ qua "." và ".."
        if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;

        // Tạo path mới
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;

        // Gọi đệ quy
        tree(buf, depth + 1);
    }

    close(fd);
}

int
main(int argc, char *argv[])
{
    if(argc < 2){
        tree(".", 0);
    } else {
        tree(argv[1], 0);
    }
    exit(0);
}