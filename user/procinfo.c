#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/procinfo.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct procinfo info;

  if(procinfo(1, &info) < 0){
    printf("procinfo failed\n");
    exit(1);
  }

  printf("PID: %d\n", info.pid);
  printf("PPID: %d\n", info.ppid);
  printf("State: %d\n", info.state);
  printf("Memory: %ld bytes\n", info.sz);
  printf("Name: %s\n", info.name);

  exit(0);
}