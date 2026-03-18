#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int mask;

  if(argc < 3){
    fprintf(2, "usage: trace mask command\n");
    exit(1);
  }

  mask = atoi(argv[1]);

  trace(mask);

  exec(argv[2], &argv[2]);

  printf("trace: exec failed\n");

  exit(0);
}