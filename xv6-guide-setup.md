# xv6 OS Project

## Overview

- **2 User Programs**: xargs, tree
- **2 System Calls**: tracing, procinfo

---

## Setup Trước Khi Bắt Đầu

### 1. Clone và Setup Repository

```bash
# Clone repo từ MIT
git clone git://g.csail.mit.edu/xv6-labs-2024
cd xv6-labs-2024

# Đổi remote thành GitHub của bạn
git remote set-url origin https://github.com/your-username/your-repo.git

# Test setup
make qemu
# Nhấn Ctrl-a x để thoát
```

### 2. Hiểu Cấu Trúc Thư Mục

```
xv6-labs/
├── kernel/          # Code hạt nhân (syscall.c, syscall.h, sysproc.c...)
├── user/            # User programs (xargs.c, ls.c, cat.c...)
├── Makefile         # Build system
└── ...
```

---

## BÀI TẬP 1: XARGS (User Program)

### Mục Tiêu
Tạo một chương trình `xargs` - đọc dữ liệu từ stdin và thực thi lệnh với dữ liệu đó.

### Ví Dụ
```bash
$ echo hello too | xargs echo bye
bye hello too

$ (echo 1; echo 2) | xargs -n 1 echo
1
2
```

### Các Bước Thực Hiện

#### Step 1: Tạo file `user/xargs.c`

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int main(int argc, char *argv[]) {
    // argv[0] = "xargs"
    // argv[1..argc-1] = command and its arguments
    
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }
    
    char *command = argv[1];
    char *cmd_argv[MAXARG];
    
    // Copy base command
    for (int i = 1; i < argc; i++) {
        cmd_argv[i - 1] = argv[i];
    }
    
    char line[512];
    int line_len = 0;
    
    // Đọc từng character từ stdin
    while (read(0, &line[line_len], 1) > 0) {
        if (line[line_len] == '\n') {
            line[line_len] = '\0';
            
            // Thêm dòng này vào argv
            cmd_argv[argc - 1] = line;
            cmd_argv[argc] = 0;
            
            // Fork và exec
            int pid = fork();
            if (pid == 0) {
                // Child process
                execv(command, cmd_argv);
                exit(1);
            } else if (pid > 0) {
                // Parent process - wait for child
                wait(0);
            } else {
                fprintf(2, "fork failed\n");
                exit(1);
            }
            
            line_len = 0;
        } else {
            line_len++;
        }
    }
    
    exit(0);
}
```

#### Step 2: Thêm vào Makefile

Tìm dòng `UPROGS =` và thêm:
```makefile
$U/_xargs\
```

#### Step 3: Build & Test

```bash
make qemu

# Trong QEMU shell
$ echo hello | xargs echo
hello

$ echo 1 2 3 | xargs echo
1 2 3
```

---

## BÀI TẬP 2: TREE (User Program)

### Mục Tiêu
Hiển thị cấu trúc thư mục dạng cây, tương tự lệnh `tree` trên Linux.

### Ví Dụ Output
```bash
$ tree a
a/
  b
  aa/
    b
    c
  ab/
    d
```

### Các Bước Thực Hiện

#### Step 1: Tạo file `user/tree.c`

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void tree(char *path, int depth);

int main(int argc, char *argv[]) {
    char *path = ".";
    
    if (argc > 1) {
        path = argv[1];
    }
    
    tree(path, 0);
    exit(0);
}

void tree(char *path, int depth) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }
    
    struct dirent de;
    struct stat st;
    char buf[512], *p;
    
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
        fprintf(2, "tree: path too long\n");
        close(fd);
        return;
    }
    
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    
    // Đầu tiên in tên thư mục ở depth này
    if (depth == 0) {
        printf("%s/\n", path);
    }
    
    // Đọc các entries trong thư mục
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
        
        // Tạo full path
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        
        if (stat(buf, &st) < 0) {
            printf("tree: cannot stat %s\n", buf);
            continue;
        }
        
        // In indentation
        for (int i = 0; i < depth + 1; i++) {
            printf("  ");  // 2 spaces per level
        }
        
        // In tên file/directory
        if (st.type == T_DIR) {
            printf("%s/\n", de.name);
            // Recursively print subdirectories
            tree(buf, depth + 1);
        } else {
            printf("%s\n", de.name);
        }
    }
    
    close(fd);
}
```

#### Step 2: Thêm vào Makefile

```makefile
$U/_tree\
```

#### Step 3: Build & Test

```bash
make qemu

# Tạo test structure
$ mkdir a
$ echo > a/b
$ mkdir a/aa
$ echo > a/aa/b
$ echo > a/aa/c
$ mkdir a/ab
$ echo > a/ab/d

# Test tree
$ tree a
$ tree .
$ tree /
```

---

## BÀI TẬP 3: TRACING (System Call)

### Mục Tiêu
Thêm system call `trace` để theo dõi các system call khác.

### Cách Hoạt Động
```bash
$ trace 32 grep hello README
# Trace chỉ read syscall (32 = 1<<SYS_read)

$ trace 2147483647 grep hello README
# Trace tất cả syscall
```

### Các Bước Thực Hiện

#### Step 1: Thêm System Call Number

**File: `kernel/syscall.h`**

Thêm ở cuối các define SYS_*:
```c
#define SYS_trace  22
```

#### Step 2: Thêm Function Pointer

**File: `kernel/syscall.c`**

Thêm trong mảng `syscalls[]`:
```c
extern uint64 sys_trace(void);

static uint64 (*syscalls[])(void) = {
    // ... existing syscalls ...
    [SYS_trace]   sys_trace,
};
```

Thêm mảng tên syscall (để in ra):
```c
static char *syscall_names[] = {
    "fork",
    "exit",
    "wait",
    "pipe",
    "read",
    "kill",
    "exec",
    "fstat",
    "chdir",
    "dup",
    "getpid",
    "sbrk",
    "sleep",
    "uptime",
    "open",
    "write",
    "mknod",
    "unlink",
    "link",
    "mkdir",
    "close",
    "trace",
};
```

Sửa hàm `syscall()` để in trace output:
```c
void
syscall(void)
{
    int num = p->trapframe->a7;
    if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        // Gọi syscall
        p->trapframe->a0 = syscalls[num]();
        
        // In trace info nếu bit được set
        if((p->trace_mask >> num) & 1) {
            printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], p->trapframe->a0);
        }
    } else {
        printf("%d %s: unknown sys call %d\n",
                p->pid, p->name, num);
        p->trapframe->a0 = -1;
    }
}
```

#### Step 3: Implement System Call

**File: `kernel/sysproc.c`**

Thêm function:
```c
uint64
sys_trace(void)
{
    int mask;
    if(argint(0, &mask) < 0)
        return -1;
    myproc()->trace_mask = mask;
    return 0;
}
```

#### Step 4: Thêm trace_mask vào Process Struct

**File: `kernel/proc.h`**

Thêm vào `struct proc`:
```c
struct proc {
    // ... existing fields ...
    int trace_mask;  // Thêm dòng này
};
```

#### Step 5: Sửa fork() để Copy trace_mask

**File: `kernel/proc.c`**

Trong hàm `fork()`, khi tạo child process:
```c
// Copy trace mask từ parent
np->trace_mask = p->trace_mask;
```

#### Step 6: Thêm User-Space Stub

**File: `user/usys.pl`**

Thêm ở cuối:
```perl
entry("trace");
```

**File: `user/user.h`**

Thêm:
```c
int trace(int);
```

#### Step 7: Build & Test

```bash
make qemu

# Test trace
$ trace 32 grep hello README

# Hoặc nếu trace program đã được provided:
$ ./trace 32 grep hello README
```

---

## BÀI TẬP 4: PROCINFO (System Call)

### Mục Tiêu
Thêm system call `procinfo` để lấy thông tin về process.

### Cấu Trúc Dữ Liệu
```c
struct procinfo {
    int pid;           // Process ID
    int ppid;          // Parent process ID
    int state;         // Process state
    uint64 sz;         // Memory size
    char name[16];     // Process name
};
```

### Các Bước Thực Hiện

#### Step 1: Tạo Struct procinfo

**File: `kernel/param.h` hoặc tạo `kernel/procinfo.h`**

```c
#ifndef _PROCINFO_H_
#define _PROCINFO_H_

struct procinfo {
    int pid;
    int ppid;
    int state;
    uint64 sz;
    char name[16];
};

#endif
```

#### Step 2: Thêm System Call Number

**File: `kernel/syscall.h`**

```c
#define SYS_procinfo  23
```

#### Step 3: Implement sys_procinfo

**File: `kernel/sysproc.c`**

```c
uint64
sys_procinfo(void)
{
    int pid;
    uint64 addr;
    
    if(argint(0, &pid) < 0)
        return -1;
    if(argaddr(1, &addr) < 0)
        return -1;
    
    // Tìm process với pid
    struct proc *p;
    for(p = proc; p < &proc[NPROC]; p++) {
        if(p->pid == pid)
            break;
    }
    
    if(p == &proc[NPROC])
        return -1;  // Process not found
    
    struct procinfo info;
    info.pid = p->pid;
    info.ppid = p->parent->pid;
    info.state = p->state;
    info.sz = p->sz;
    safecpy(info.name, p->name, sizeof(info.name));
    
    // Copy to user space
    if(copyout(myproc()->pagetable, addr, (char *)&info, sizeof(info)) < 0)
        return -1;
    
    return 0;
}
```

#### Step 4: Thêm vào syscall.c

Thêm extern và function pointer:
```c
extern uint64 sys_procinfo(void);

static uint64 (*syscalls[])(void) = {
    // ...
    [SYS_procinfo] sys_procinfo,
};
```

Thêm vào mảng syscall_names:
```c
"procinfo",  // 23
```

#### Step 5: Thêm User-Space Stub

**File: `user/usys.pl`**

```perl
entry("procinfo");
```

**File: `user/user.h`**

```c
struct procinfo;
int procinfo(int, struct procinfo*);
```

#### Step 6: Test

```bash
make qemu

# Tạo test program hoặc sử dụng directly
$ procinfo $$  # Lấy info của current process
```

---

## Cách Test Toàn Bộ

```bash
# Build
make clean
make qemu

# Trong QEMU, test từng bài:

# Test xargs
$ echo hello | xargs echo
$ (echo 1; echo 2; echo 3) | xargs echo

# Test tree
$ tree a
$ tree .

# Test trace (nếu trace program có sẵn)
$ trace 32 grep hello README

# Test procinfo
$ procinfo $$
```

---

## Tips & Tricks

1. **Sử dụng git để track changes**:
```bash
git add -A
git commit -m "Add xargs implementation"
git diff origin/util > my_changes.patch
```

2. **Debug bằng cách in ra**:
```c
printf("DEBUG: value = %d\n", value);
```

3. **Xem syscall numbers**:
```bash
grep "^#define SYS_" kernel/syscall.h
```

4. **Compile lỗi? Xem Makefile error message kỹ**:
- Missing files?
- Lỗi syntax?
- Undefined reference?

5. **QEMU hang? Nhấn Ctrl-p để xem process, hoặc Ctrl-a x để thoát**

---

## Checklist Submit

- [ ] xargs hoạt động đúng
- [ ] tree hiển thị cấu trúc đúng
- [ ] trace in ra đúng syscall
- [ ] procinfo trả về thông tin đúng
- [ ] Mọi file được thêm vào Makefile
- [ ] Git diff patch được tạo
- [ ] Report PDF được viết

---

## Tài Liệu Tham Khảo

1. xv6 Book: https://pdos.csail.mit.edu/6.1810/2024/xv6/book-riscv-rev3.pdf
2. MIT Lab Page: https://pdos.csail.mit.edu/6.1810/2024/labs/util.html
3. Syscall Lab: https://pdos.csail.mit.edu/6.1810/2024/labs/syscall.html