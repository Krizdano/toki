#ifndef HELP_COMMAND_H
#define HELP_COMMAND_H

typedef struct {
  const char *name;
  int (*function)(void);
}HelpArgs;


int CommandHelp(const char *option);
int usageNew(void);
int usageRun(void);
int usageGen(void);
int usage(void);

#endif // !HELP_COMMAND_H
