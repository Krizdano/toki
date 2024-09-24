#include <stdio.h>
#include <string.h>

#include "commands/inc/project_initializer.h"
#include "commands/inc/build_run_clean_debug.h"
#include "commands/inc/generate_files.h"
#include "commands/inc/help-command.h"
#include "commands/inc/options.h"


typedef struct {
  const char *name;
  void (*function)(const char *argv[]);
} Args;

int main(int argc, char *argv[]) {

  if (argc < 2) {
    usage();
    return 1;
  }

  CheckForOptions(&argc, argv);

  Args commands[] = {
    {"new", newProject},
    {"run", performOperation},
    {"build", performOperation},
    {"clean", performOperation},
    {"debug", performOperation},
    {"gen", GenerateFiles},
  };

  size_t number_of_commands = sizeof(commands)/sizeof(commands[0]);
  for(size_t i = 0; i < number_of_commands; i++) {
    if (strcmp(argv[1], commands[i].name) == 0) {
      commands[i].function( (const char**) argv);
      return 0;
    }
  }
  printf("%s is not a valid command\n", argv[1]);
  usage();
  return 2;
}
