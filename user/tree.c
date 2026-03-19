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

    if((fd = open(path, 0)) < 0){
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if(st.type != T_DIR){
        for(int i = 0; i < depth; i++)
            printf("  ");
        printf("%s\n", path);
        close(fd);
        return;
    }

    for(int i = 0; i < depth; i++)
        printf("  ");
    printf("%s/\n", path);

    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;

        if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;

        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;

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