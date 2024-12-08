#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include "inc/generate_files.h"
#include "inc/options.h"

const int MAX_PATH_SIZE = 1024;

/**
 * GenerateFiles - Generates files based on the provided command line arguments.
 *
 * @argv: An array of command line arguments. The expected format is:
 *        argv[0] - program name
 *        argv[1] - command (should be "gen")
 *        argv[2] - file type (e.g., "flake" or "makefile")
 *        argv[3] - type of the file to generate (e.g., "c", "asm", etc.)
 *
 * This function checks the validity of the arguments and calls the appropriate
 * file generation function based on the specified file type.
 */
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
/**
 * GenFlake - Generates a flake.nix file based on the specified type and packages.
 *
 * @directory: The directory where the flake.nix file will be created.
 * @type: The type of flake to generate (e.g., "c", "asm", "fasm", "generic").
 * @packages: A string containing additional packages to include in the flake.
 *
 * This function constructs the contents of a flake.nix file and calls
 * createFile to write it to the specified directory.
 */
void GenFlake(const char directory[], const char type[], const char packages[]) {

  FileSpec file[] = {
    {"c", "flake.nix", "gdb gcc gnumake"},
    {"asm", "flake.nix", "nasm gdb gnumake"},
    {"fasm", "flake.nix", "fasm gdb gnumake"},
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

/**
 * GenMain - Generates a main source file based on the specified type.
 *
 * @directory: The directory where the main source file will be created.
 * @type: The type of source file to generate (e.g., "c", "asm", "fasm").
 *
 * This function constructs the contents of a main source file and calls
 * createFile to write it to the specified directory.
 */
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

  mainfile.fasm =
    "format ELF64 executable 3\n"
    "entry start\n"
    "\n"
    "segment readable executable\n"
    "start:\n"
    "       mov  rax, 1\n"
    "       mov  rdi, 1\n"
    "       mov  rsi, hello\n"
    "       mov  rdx, msg_size\n"
    "       syscall\n"
    "\n"
    "       mov rax, 60\n"
    "       mov rdi, 0\n"
    "       syscall\n"
    "\n"
    "segment readable writable\n"
    "hello db \"Hello world!\",10\n"
    "msg_size = $ - hello\n"
      ;

  FileSpec file[] = {
    {"c", "main.c", mainfile.c },
    {"asm", "main.asm", mainfile.assembly },
    {"fasm", "main.asm", mainfile.fasm },
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
/**
 * GenMake - Generates a Makefile based on the specified type.
 *
 * @directory: The directory where the Makefile will be created.
 * @type: The type of Makefile to generate (e.g., "c", "asm", "fasm").
 *
 * This function constructs the contents of a Makefile and calls
 * createFile to write it to the specified directory.
 */
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

  makefile.fasm =
    "AS=fasm\n"
    "LD=ld\n"
    "AFLAGS=-s\n"
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
    "$(BIN): $(SRCS)\n"
    "\t@mkdir -p $(dir $@)\n"
    "\t$(AS) $< $(BIN)\n"
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
    {"fasm", "Makefile", makefile.fasm},
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

/* CreatDirectory - Create a directory at the specified path.
 * @root_dir: The root directory where the new directory will be created.
 *           If NULL, the new directory will be created in the current working directory.
 * @directory_name: The name of the directory to create.
 * This function constructs the full path for the new directory and attempts to create it
 * with read, write, and execute permissions for the user, group, and others.
 * If the directory creation fails, an error message is printed to stderr and the program exits.
 */
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

/* CreateFile - Create a file in the specified directory with the given content.
 * @root_dir: The root directory where the file will be created.
 *           If NULL, the file will be created in the specified sub_directory.
 * @sub_directory: The sub-directory where the file will be created.
 * @filename: The name of the file to create.
 * @permission: The file permission mode (e.g., "w" for write).
 * @content: The content to write to the file.
 * This function constructs the full path for the new file and attempts to create it.
 * If the file creation fails, an error message is printed to stderr and the program exits.
 * The content is written to the file before closing it.
 */
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
