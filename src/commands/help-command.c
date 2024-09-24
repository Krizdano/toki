#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "inc/help-command.h"


int CommandHelp(const char *option) {
  HelpArgs options[] = {
    { "new", usageNew },
    { "run", usageRun },
    { "gen", usageGen },
  };

  size_t options_length = sizeof(options)/sizeof(options[0]);
  for(size_t i = 0; i < options_length; i++) {
    if(strcmp(option, options[i].name) == 0) {
      options[i].function();
      return 0;
    }
  }
  usage();

  return 0;
}

int usageNew(void) {
  fprintf(stdout, "\nusage: toki new [flags] [projectname]\n");
  fprintf(stdout, "\nflags:\n");
  fprintf(stdout, " -a                - create an assembly project\n\n");
  return 0;
}

int usageRun(void) {
  fprintf(stdout, "\nusage: toki run [arguments]\n");
  fprintf(stdout, "\narguments will be passed to the program as commandline arguments\n");
  fprintf(stdout, "\neg. toki run arg\n\n");
  fprintf(stdout, "if there is more than one argument:\n");
  fprintf(stdout, "\ntoki run \"arg1 arg2\"\n\n");
  return 0;
}

int usageGen(void) {
  fprintf(stdout, "\nusage: toki gen [filetype] [options] [arguments]\n");
  fprintf(stdout, "\nFiletypes:\n");
  fprintf(stdout, " makefile             - generate a makefile with given option\n");
  fprintf(stdout, " flake                - generate a flake.nix with given option\n");
  fprintf(stdout, "\noptions:\n");
  fprintf(stdout, " c             - generates a makefile or flake.nix to build a c project\n");
  fprintf(stdout, " asm           - generates a makefile or flake.nix to build a assembly project\n");
  fprintf(stdout, " with          - generates a flake.nix with provided arguments\n");
  fprintf(stdout, "\neg. toki gen flake with [package]\n\n");
  fprintf(stdout, "if there is more than one argument:\n");
  fprintf(stdout, "\ntoki gen flake with \"package1 package2\"\n\n");
  return 0;
}

int usage(void) {
  fprintf(stdout, "\nusage: toki [command] [option]\n");
  fprintf(stdout, "\nThese are the commands that can be used:\n");
  fprintf(stdout, " new               - creates new projects ( new -a for assembly project )\n");
  fprintf(stdout, " build             - builds the project using Makefile\n");
  fprintf(stdout, " run               - runs the project using make\n");
  fprintf(stdout, " clean             - cleans up the obj and binary files from build folder\n");
  fprintf(stdout, " debug             - opens gdb for debugging\n");
  fprintf(stdout, " gen               - generate a flake.nix or Makefile\n");
  fprintf(stdout, "\nRun <command> --help for more information\n");
  return 0;
}
