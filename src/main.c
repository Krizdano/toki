#include <stdio.h>
#include <string.h>

#include "commands/inc/project_initializer.h"
#include "commands/inc/build_run_clean_debug.h"
#include "commands/inc/generate_files.h"
#include "commands/inc/help-command.h"
#include "commands/inc/options.h"

/*
 * struct Args - Structure to hold command names and their corresponding function pointers.
 * @name: Command name.
 * @function: Function pointer to execute the command.
 *
 * This structure associates a command name (as a string) with a function that
 * executes the command. The function takes an array of command-line arguments.
 */
typedef struct {
  const char *name;
  void (*function)(const char *argv[]);
} Args;

/*
 * main - Main entry point of the command-line application.
 * @argc - The number of command-line arguments.
 * @argv - The array of command-line arguments.
 *
 * This function processes command-line arguments, checks for valid commands,
 * and invokes the corresponding functions. If no valid command is provided,
 * it displays usage information.
 *
 * Return: Exit status: 0 on success, 1 if no command is provided, or 2 if an invalid command is given.
*/

int main(int argc, char *argv[]) {

  if (argc < 2) {
    usage();
    return 1;
  }

  /* Check for any command-line options and adjust argc and argv accordingly */
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
      commands[i].function( (const char**) argv);  /* Execute the command */
      return 0;
    }
  }
  /* If no valid command is found, notify the user and display usage information */
  printf("%s is not a valid command\n", argv[1]);
  usage();
  return 2;
}
