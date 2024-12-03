#ifndef GENERATE_FILES_H
#define GENERATE_FILES_H

extern const int MAX_PATH_SIZE;

void GenFlake(const char directory[], const char type[], const char packages[]);
void GenMain(const char directory[], const char type[]);
void GenMake(const char directory[], const char type[]);
void GenerateFiles(const char *argv[]);

void createDirectory(const char root_dir[], const char directory_name[]);
void createFile(const char root_dir[], const char sub_directory[], const char filename[], const char permission[], const char content[]);

typedef struct {
  const char *type;
  const char *name;
  const char *content;
}FileSpec;

typedef struct {
  const char *assembly;
  const char *c;
  const char *fasm;
} Content;

#endif // !GENERATE_FILES_H
