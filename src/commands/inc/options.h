#ifndef _OPTIONS_H
#define _OPTIONS_H

typedef struct {
  int assembly;
  int flake;
  int directory;
  int assembler;
  int pass_to_make;
} Flag;

typedef struct {
  char *flake_packages;
  char *arguments_for_make;
  char *new_directory;
  char *assembler;
} Options;

extern Flag flag;
extern Options options;

void CheckForOptions(int *argc, char *argv[]);

#endif // !_OPTIONS_H
