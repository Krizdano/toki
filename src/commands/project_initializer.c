#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "inc/project_initializer.h"
#include "inc/generate_files.h"
#include "inc/options.h"

const size_t MAX_PROJECT_LENGTH = 50;

// create new projects
void newProject(const char *argv[]) {

  const char *project_name = argv[2];
  const char *cwd = ".";
  char *type = NULL;

  if (project_name == NULL) {
    fprintf(stderr, "Error: no name provided for the project\n");
    exit(1);
  }

  if (strlen(project_name) > MAX_PROJECT_LENGTH) {
    fprintf(stderr, "whoa there!..\nThat's a really long name don't you think?\n");
    exit(1);
  };

  if(options.assembler != NULL) {
    type = options.assembler;
  }
  else if(flag.assembly) {
    type = "asm";
  }
  else {
    type = "c";
  }

  createDirectory(cwd, project_name);

  if(chdir(project_name) == -1) {
    fprintf(stderr, "ERROR: changing to directory %s: %s", project_name, strerror(errno));
    exit(-1);
  }

  char subdirectories[][6] = {"src", "build", "bin", "obj"};

  size_t number_of_subdirs = sizeof(subdirectories) / sizeof(subdirectories[0]);
  for (size_t i = 0; i < number_of_subdirs; i ++) {
    char path[sizeof(subdirectories) + MAX_PROJECT_LENGTH];
    if (i == 2 || i == 3) {
      snprintf(path, sizeof(path), "%s/%s", subdirectories[1], subdirectories[i]);
    }
    else {
      snprintf(path, sizeof(path), "%s", subdirectories[i]);
    }
    createDirectory(cwd, path);
  }

  GenMake(cwd, type);
  GenMain(cwd, type);

#ifdef FLAKE_SUPPORT
  GenFlake(".", type, NULL);
  EnableDirenv();
#endif /* ifdef FLAKE_SUPPORT */

  fprintf(stdout, "created new project %s\n", project_name);

  initGit();
}

#ifdef FLAKE_SUPPORT
void EnableDirenv(void) {

  char direnv_allow[] = "direnv allow";
  char *cwd = ".";

  char direnv_content[] = {
    "watch_file flake.nix flake.lock\n"
    "use flake"
  };

  FileSpec file = {"direnv", ".envrc", direnv_content};

  createFile(NULL, cwd, file.name, "w", file.content);

  if(system(direnv_allow) == -1) {
    perror("direnv error\n");
    exit(1);
  }
}
#endif /* ifdef FLAKE_SUPPORT */

void initGit(void) {

  char gitinit[] = "git init -q -b main";

  char *cwd = ".";

  char gitignore_content[] = {
    "/build\n"
    "/.direnv"
  };

  FileSpec file = { "git", ".gitignore", gitignore_content };

  if(system(gitinit) == -1) {
    perror("Error initializing Git repository\n");
    exit(1);
  }

#ifdef FLAKE_SUPPORT
  char gitadd[] = "git add flake.nix";

  if(system(gitadd) == -1) {
    perror("Error adding flake.nix to git repo\n");
    exit(1);
  }
#endif /* ifdef FLAKE_SUPPORT */

  createFile(NULL, cwd, file.name, "w", file.content);
}
