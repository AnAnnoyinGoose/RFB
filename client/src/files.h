typedef struct {
  char *path, *size, *type, *name;
} File;

typedef struct {
  File *files;
  int num_files;
  int capacity;
} Files;

void print_file(File file);
void new_file(File *file, const char *path, const char *size, const char *type);
void free_file(File *file);


void init_files_vector(Files *files);
void append_file(Files *files, File file);
void get_files_from_server(Files *files);

void set_file_socket_fd(int fd);
void set_file_name(File *file);

