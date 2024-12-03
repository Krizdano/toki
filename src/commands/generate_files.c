#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include "inc/generate_files.h"
#include "inc/options.h"

const int MAX_PATH_SIZE = 1024;

void GenerateFiles(const char *argv[]) {

  const char *file_type = argv[2];
  const char *directory = ".";
  const char *type = argv[3];

#ifdef FLAKE_SUPPORT
  const char *packages = NULL;

  if(flag.flake) {
    packages = options.flake_packages;
    type = "generic";
  }
#endif /* ifdef FLAKE_SUPPORT */

  if (file_type == NULL) {
    fprintf(stderr,"Error: does not have enough arguments\n");
    fprintf(stderr,"\nUSAGE:\n");
    fprintf(stderr,"\ttoki gen [FILENAME] <TYPE>\n");
    fprintf(stderr,"\nrun toki gen --help for more information\n");
    exit(1);
  }


  if(type == NULL) {
    fprintf(stderr,"ERROR: toki gen %s does not have enough arguments\n", file_type);
    fprintf(stderr,"\nUSAGE:\n");
    fprintf(stderr,"\ttoki gen [FILENAME] <TYPE>\n");
    fprintf(stderr,"\nrun toki gen --help for more information\n");
    exit(1);
  }

  if(flag.directory) {
    directory = options.new_directory;
  }

  if(strcmp(file_type, "flake") == 0) {

#ifdef FLAKE_SUPPORT
    GenFlake(directory, type, packages);
#endif /* ifdef FLAKE_SUPPORT */

#ifndef FLAKE_SUPPORT
    fprintf(stderr, "flake support is not enabled\n");
    exit(1);
#endif /* ifndef FLAKE_SUPPORT */

  }
  else if(strcmp(file_type, "makefile") == 0) {
    GenMake(directory, type);
  }
  else {
    fprintf(stderr, "GenError: unkown filename %s\n", file_type);
    fprintf(stderr, "\n\tsupported filenames: \n");
    printf(" makefile             - generate a makefile with given option\n");
    printf(" flake                - generate a flake.nix with given option\n");
    fprintf(stderr,"\nrun toki gen --help for more information\n");
    exit(1);
  }
}

#ifdef FLAKE_SUPPORT
void GenFlake(const char directory[], const char type[], const char packages[]) {

  FileSpec file[] = {
    {"c", "flake.nix", "gdb gcc gnumake"},
    {"asm", "flake.nix", "nasm gdb gnumake"},
    {"generic", "flake.nix", packages },
  };

  size_t lang_length = sizeof(file) / sizeof(file[0]);

  for (size_t i = 0; i < lang_length; i++) {
    if (strcmp(type, file[i].type) == 0) {

      char flake_contents[1024];
      snprintf(flake_contents, sizeof(flake_contents),
               "{\n"
               "  description = \"Auto generated nix shell using toki\";\n"
               "\n"
               "  inputs.nixpkgs.url = \"github:nixos/nixpkgs?ref=nixos-unstable\";\n"
               "\n"
               "  outputs = { self, nixpkgs, ... }:\n"
               "   let\n"
               "     supportedSystems = [\n"
               "       \"x86_64-linux\"\n"
               "       \"i686-linux\"\n"
               "       \"aarch64-linux\"\n"
               "     ];\n"
               "     forAllSystems =\n"
               "       f:\n"
               "      nixpkgs.lib.genAttrs supportedSystems (\n"
               "        system:\n"
               "        f {\n"
               "          pkgs = nixpkgs.legacyPackages.${system};\n"
               "          inherit system;\n"
               "        });\n"
               "   in\n"
               "     {\n"
               "       devShells = forAllSystems (\n"
               "         { pkgs, system, ... }: {\n"
               "           default = pkgs.stdenvNoCC.mkDerivation {\n"
               "             name = \"toki-build-shell\";\n"
               "             buildInputs = with pkgs; [ %s ];\n"
               "           };\n"
               "         });\n"
               "     };\n"
               "}",
               file[i].content);

      createFile(NULL, directory, file[i].name, "w", flake_contents);
      return;
    }
  }

  fprintf(stderr, "error generating flake, unknown option %s \n", type);
  exit(1);
}
#endif /* ifdef FLAKE_SUPPORT */

void GenMain(const char directory[], const char type[]) {

  Content mainfile = {NULL};

  mainfile.c =
    "#include <stdio.h>\n"
    "\n"
    "int main(void) {\n"
    "  printf(\"Hello, World!\\n\");\n"
    "}\n"
    ;

  mainfile.assembly =
    "\tsys_write equ 1\n"
    "\tstdout equ 1\n"
    "\n"
    "\tglobal _start\n"
    "\n"
    "\t%macro print 2\n"
    "\tmov rax, sys_write\n"
    "\tmov rdi, stdout\n"
    "\tmov rsi, %1\n"
    "\tmov rdx, %2\n"
    "\tsyscall\n"
    "\t%endmacro\n"
    "\n"
    "\tsection .data\n"
    "\tgreet db \"hello world\", 10\n"
    "\tlen equ $ - greet\n"
    "\n"
    "\tsection .text\n"
    "_start:\n"
    "\tprint greet, len\n"

    "_exit:\n"
    "\tmov rax, 60\n"
    "\tmov rdi, 0\n"
    "\tsyscall\n"
    ;

  FileSpec file[] = {
    {"c", "main.c", mainfile.c },
    {"asm", "main.asm", mainfile.assembly },
  };

  size_t file_length = sizeof(file)/sizeof(file[0]);

  for(size_t i = 0; i < file_length; i++) {
    if(strcmp(type, file[i].type) == 0) {
      createFile(directory, "src", file[i].name, "w", file[i].content);
      return;
    }
  }

  fprintf(stderr, "error generating mainfile: unknown option %s", type);
  exit(1);
}


void GenMake(const char directory[], const char type[]) {

  Content makefile = {NULL};

  makefile.c =
    "CC=cc\n"
    "CFLAGS=-g -Wall -Wextra -pedantic\n"
    "OBJ=build/obj\n"
    "PWD=$(shell pwd)\n"
    "ELF=$(shell basename $(PWD))\n"
    "BIN=build/bin/$(ELF)\n"
    "SRCS=$(shell find src/ -type f -name '*.c')\n"
    "OBJS=$(patsubst src/%.c, build/obj/%.o, $(SRCS))\n"
    "RM=rm -rf\n"
    "\n"
    "default: all\n"
    "\n"
    "install: all\n"
    "\n"
    "build: all\n"
    "\n"
    "debug: all\n"
    "\tgdb $(BIN)\n"
    "\n"
    "run: all\n"
    "\t@./$(BIN) $(filter-out $@,$(MAKECMDGOALS))\n"
    "\n"
    "all: $(BIN)\n"
    "\n"
    "$(BIN): $(OBJS)\n"
    "\t$(CC) $(CFLAGS) $(OBJS) -o $@\n"
    "\n"
    "$(OBJ)/%.o: src/%.c\n"
    "\t@mkdir -p $(dir $@)\n"
    "\t$(CC) $(CFLAGS) -c $< -o $@\n"
    "\n"
    "clean:\n"
    "\t$(RM) build/bin/* build/obj/*\n"
    "\n"
    "%:\n"
    "\t@:\n"
    ;

  makefile.assembly =
    "AS=nasm\n"
    "LD=ld\n"
    "AFLAGS=-f elf64 -g\n"
    "LFLAGS=-m elf_x86_64\n"
    "OBJ=build/obj\n"
    "PWD=$(shell pwd)\n"
    "ELF=$(shell basename $(PWD))\n"
    "BIN=build/bin/$(ELF)\n"
    "SRCS=$(shell find src/ -type f -name '*.asm')\n"
    "OBJS=$(patsubst src/%.asm, build/obj/%.o, $(SRCS))\n"
    "RM=rm -rf\n"
    "\n"
    "default: all\n"
    "\n"
    "install: all\n"
    "\n"
    "build: all\n"
    "\n"
    "debug: all\n"
    "\tgdb $(BIN)\n"
    "\n"
    "run: all\n"
    "\t@./$(BIN) $(filter-out $@,$(MAKECMDGOALS))\n"
    "\n"
    "all: $(BIN)\n"
    "\n"
    "$(BIN): $(OBJS)\n"
    "\t$(LD) $(LFLAGS) $(OBJS) -o $@\n"
    "\n"
    "$(OBJ)/%.o: src/%.asm\n"
    "\t@mkdir -p $(dir $@)\n"
    "\t$(AS) $(AFLAGS) $< -o $@\n"
    "\n"
    "clean:\n"
    "\t$(RM) build/bin/* build/obj/*\n"
    "\n"
    "%:\n"
    "\t@:\n"
    ;

  FileSpec file[] = {
    {"c", "Makefile", makefile.c},
    {"asm", "Makefile", makefile.assembly},
  };


  size_t file_length = sizeof(file)/sizeof(file[0]);

  for(size_t i = 0; i < file_length; i++) {
    if(strcmp(type, file[i].type) == 0 ) {
      createFile(NULL, directory, file[i].name, "w", file[i].content);
      return;
    }
  }

  fprintf(stderr, "unsupported language for makefile: %s\n", type);
  exit(1);
}


void createDirectory(const char root_dir[], const char directory_name[]) {

  char path[MAX_PATH_SIZE];

  if (root_dir == NULL) {
    snprintf(path, sizeof(path), "%s", directory_name);
  } else {
    snprintf(path, sizeof(path), "%s/%s", root_dir, directory_name);
  }

  if (mkdir( path, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
    fprintf(stderr, "Error creating directory %s: %s\n", directory_name, strerror(errno));
    exit(1);
  }
}

void createFile(const char root_dir[], const char sub_directory[], const char filename[], const char permission[], const char content[]) {

  char path[MAX_PATH_SIZE];

  if (root_dir == NULL) {
    snprintf(path, sizeof(path), "%s/%s", sub_directory, filename);
  } else {
    snprintf(path, sizeof(path), "%s/%s/%s", root_dir, sub_directory, filename);
  }

  FILE *file = fopen(path, permission);

  if (file == NULL) {
    fprintf(stderr, "Error creating file %s: %s\n", filename, strerror(errno));
    exit(1);
  }

  fprintf(file, "%s", content);

  fclose(file);
}
