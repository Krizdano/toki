#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "inc/options.h"
#include "inc/help-command.h"
#include "inc/options.h"

Flag flag = {0};
Options options = {NULL};

void CheckForOptions(int *argc, char *argv[]) {

  int opt;
  int option_index = 0;

  static struct option longoption[] = {
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {"with", required_argument , 0, 'w'},
    {"directory", required_argument , 0, 'd'},
    {"assembler", required_argument , &flag.assembler, 1},
    {0, 0, 0, 0}
  };

  while((opt = getopt_long(*argc, argv, "vhaw:d:p:", longoption, &option_index)) != -1) {
    switch(opt) {
    case 'v':
      fprintf(stdout, "version - 0.2\n");
      exit(0);
    case 'h':
      CommandHelp(argv[1]);
      exit(0);
    case 'a':
      flag.assembly = 1;
      break;
    case 'w':
      flag.flake = 1;
      // flake_packages = optarg;
      options.flake_packages = optarg;
      break;
    case 'd':
      flag.directory = 1;
      // optdirectory = optarg;
      options.new_directory = optarg;
      break;
    case 'p':
      flag.pass_to_make = 1;
      options.arguments_for_make = optarg;
      break;
    case '?':
      fprintf(stdout, "run toki --help for more information\n");
      exit(1);
    }
  }

  if(flag.assembler) {
    options.assembler = "fasm";
  }

  int i;
  int z;
  for (i = optind, z = 1; i < *argc; i++, z++) {
    argv[z] = argv[i];
  }
  *argc = z;
  argv[*argc] = NULL;

}
