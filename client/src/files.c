#include "files.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


static int socket_fd;
void set_file_socket_fd(int fd) {
  socket_fd = fd;
}


void print_file(File file) {
  printf("Path: %s\n", file.path);
  printf("Size: %s\n", file.size);
  printf("Type: %s\n", file.type);
}

void new_file(File *file, const char *path, const char *size,
              const char *type) {
  file->path = strdup(path);
  file->size = strdup(size);
  file->type = strdup(type);
  file->name = NULL;
}

void free_file(File *file) {
  free(file->path);
  free(file->size);
  free(file->type);
  free(file->name);
}

void init_files_vector(Files *files) {
  files->num_files = 0;
  files->capacity = 2;
  files->files = malloc(sizeof(File) * files->capacity);
}

void append_file(Files *files, File file) {
  if (files->num_files == files->capacity) {
    files->capacity *= 2;
    files->files = realloc(files->files, files->capacity * sizeof(File));
    assert(files->files != NULL);
  }
  files->files[files->num_files] = file;
  files->num_files++;
}

void get_files_from_server(Files *files) {
  char buffer[1024];
  size_t n = 0;
  while (1) {
    n = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
      break;
    }
    buffer[n] = '\0';

    if (strstr(buffer, "SfL") != NULL) {
      while (1) {
        n = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
          break;
        }
        buffer[n] = '\0';
        if (strstr(buffer, "EfL") != NULL) {
          return;
        }

        char *path = strtok(buffer, " \r\n ");
        char *size = strtok(NULL, " \r\n ");
        char *type = strtok(NULL, " \r\n ");

        File file;
        new_file(&file, path, size, type);
        append_file(files, file);
      }
      break;
    }
  }
}


void set_file_name(File *file) {
  file->name = strrchr(file->path, '/');
  if (file->name != NULL) {
    file->name++;
  } else {
    assert(0);
  }
};



