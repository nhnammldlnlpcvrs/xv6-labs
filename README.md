# Project OS - Xv6 Labs - Process Management & User Utilities

## Introduction

This project is part of an **xv6 operating system lab**, focusing on both:

- User-level program implementation
- Kernel-level system call development

The lab consists of **4 tasks**, and all have been completed successfully.  
This README explains **what was implemented and how**, based directly on the provided `git diff` files.

---

## Project Overview

| Task | Description |
|------|------------|
| Task 1 | Implement `xargs` |
| Task 2 | Implement `tree` |
| Task 3 | Implement `trace` system call |
| Task 4 | Implement `procinfo` system call |

---

## Build & Run

```bash
make clean
make qemu
```

## Task 1

**Description**: 
- Implement a simplified version of the UNIX xargs utility.

- The program reads input from stdin and executes a command by appending the input as arguments.

Example
```bash
$ echo hello too | xargs echo bye
bye hello too
```

## Task 2: tree
**Description**:

- Implement a simplified version of the UNIX tree command to display directory structure recursively.

Example
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

## Task 3: trace (System Call)

**Description**:

- Implement a system call trace(mask) to trace system calls of a process.

- Mask determines which syscalls to trace

- Each bit corresponds to a syscall

Example
```bash
$ trace 32 grep hello README
3: syscall read -> 1023
```

## Task 4: procinfo (System Call)

**Description**:

- Implement a system call:
```bash
int procinfo(int pid, struct procinfo *info);
```
This retrieves information about a process.

Data Structure:
```bash
struct procinfo {
  int pid;
  int ppid;
  int state;
  uint64 sz;
  char name[16];
};
```

## References

- [xv6 util lab](https://pdos.csail.mit.edu/6.1810/2024/labs/util.html)
- [xv6 syscall lab](https://pdos.csail.mit.edu/6.1810/2023/labs/syscall.html)
- [xv6 book (MIT)](https://pdos.csail.mit.edu/6.1810/2023/xv6/book-riscv-rev1.pdf)

## License

This project is based on the original xv6 operating system.

The xv6 source code is licensed under the MIT License.

This repository contains modifications and extensions for educational purposes.

See the full license in the [LICENSE](LICENSE) file.
