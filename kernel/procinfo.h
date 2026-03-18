#ifndef PROCINFO_H
#define PROCINFO_H

#include "types.h"

struct procinfo {
    int pid;
    int ppid;
    int state;
    uint64 sz;
    char name[16];
};

#endif