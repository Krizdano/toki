#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "inc/generate_files.h"
#include "inc/options.h"

void checkForMakefile(char *cwd, int buffer) {

  if (getcwd(cwd, buffer) == NULL) {
    perror("cannot get the value of current directory");
    exit(-1);
  }

  char makefile[] = "Makefile";
  char makefilepath[strlen(cwd) + sizeof(makefile) + 1];
  snprintf(makefilepath, sizeof(makefilepath), "%s/%s", cwd, makefile);

  while (access(makefilepath, F_OK) == -1) {
    if(chdir("..") == -1) {
      perror("error finding Makefile: ");
      exit(-1);
    }

    if(strcmp(cwd, "/") == 0) {
      fprintf(stderr, "Makefile file not found\n");
      exit(-1);
    }

    if (getcwd(cwd, buffer) == NULL) {
      perror("cannot get the value of current directory: ");
      exit(-1);
    }

    snprintf(makefilepath, sizeof(makefilepath), "%s/%s", cwd, makefile);

  }

}

void performOperation(const char *argv[]) {

  char cwd[MAX_PATH_SIZE];
  const char *argument1 = argv[1];
  const char *argument2 = "";

  checkForMakefile(cwd, sizeof(cwd));

  if(chdir(cwd) == -1) {
    fprintf(stderr, "cannot change to directory %s: %s", cwd, strerror(errno));
  }

  if (argv[2] != NULL) {
    argument2 = argv[2];
  }

  if((strcmp(argv[1], "build") != 0) && flag.pass_to_make) {
    fprintf(stderr,"Error: please run toki build -p to pass arguments to make\n");
    fprintf(stderr,"\nrun toki build --help for more info\n");
    exit(-1);
  }
  else if ((strcmp(argv[1], "build") == 0) && flag.pass_to_make) {
    argument1 = options.arguments_for_make;
  }

  char make[] = "make";
  char command[sizeof(make) + strlen(argument1) + strlen(argument2) + 2];
  snprintf(command, sizeof(command), "%s %s %s", make, argument1, argument2);

  if(system(command) == -1) {
    fprintf(stderr,"cannot run make: %s", strerror(errno));
  }

}
