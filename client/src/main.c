#include "main.h"

#define PORT 344
#define HOST "0.0.0.0"

typedef enum { Private, Public } Owner;

struct File {
  const char *name, *path, *mime_type, *md5, *size;
  Owner owner;
} File;

struct File *new_file() {
  struct File *f = malloc(sizeof(struct File));
  f->name = NULL;
  f->path = NULL;
  f->mime_type = NULL;
  f->md5 = NULL;
  f->size = NULL;
  f->owner = Private;
  return f;
}

void init(void) {
  const int screenWidth = 800;
  const int screenHeight = 450;
  InitWindow(screenWidth, screenHeight, "RFBv0.1nR Client");
  SetTargetFPS(60);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
}

void winclose(void) { CloseWindow(); }

int init_client_socket(void) {
  int socket_fd;
  struct sockaddr_in server_addr;
  struct hostent *server;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  assert(socket_fd != -1 && "socket() failed");
  server = gethostbyname(HOST);
  assert(server != NULL && "gethostbyname() failed");
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr,
        server->h_length);
  server_addr.sin_port = htons(PORT);
  (void)connect(socket_fd, (struct sockaddr *)&server_addr,
                sizeof(server_addr));
  assert(socket_fd != -1 && "connect() failed");
  return socket_fd;
}

int is_rfb_server(int socket_fd) {
  char buffer[1024];
#define MSG "RFBv0.1nR"
  ssize_t n = send(socket_fd, MSG, strlen(MSG), 0);
#undef MSG
  assert(n > 0 && "send() failed");
  n = recv(socket_fd, buffer, sizeof(buffer), 0);
  buffer[n] = '\0';
  if (strstr(buffer, "RFBv0.1nR") != NULL)
    return 1; // rfb server
  return 0;
}

int try_login(int socket_fd) {
  char buffer[1024];
#define MSG "RFB-L"
  ssize_t n = send(socket_fd, MSG, strlen(MSG), 0);
#undef MSG
  assert(n > 0 && "send() failed");

  n = recv(socket_fd, buffer, sizeof(buffer), 0);
  buffer[n] = '\0';
  if (strstr(buffer, "RFB-L-IN") != NULL) {
    char pswd[1024];
    printf("Enter password: ");
    fgets(pswd, 1024, stdin);
    send(socket_fd, pswd, strlen(pswd), 0);
    n = recv(socket_fd, buffer, sizeof(buffer), 0);
    buffer[n] = '\0';
    if (strstr(buffer, "RFB-L-OK") != NULL)
      return 1; // login succeeded
    return 0;
  }
  return 0;
}

struct File **get_files(int socket_fd) {
  char buffer[1024];
  ssize_t n;
  struct File **files = malloc(2 * sizeof(struct File *));
  while (true) {
    n = recv(socket_fd, buffer, sizeof(buffer), 0);
    buffer[n] = '\0';
    if (n == 0)
      continue;
    if (strstr(buffer, "SfL") != NULL)
      continue;
    if (strstr(buffer, "EfL") != NULL)
      break;
    struct File *f = new_file();
#define DELIM "\r\n" // 1st is path then size and mime
    f->path = strtok(buffer, DELIM) + 2;
    f->size = strtok(NULL, DELIM);
    f->mime_type = (strtok(NULL, DELIM));
    printf("%s %s %s\n", f->path, f->size, f->mime_type);
  }
  return files;
}

int main(void) {
  int socket_fd = init_client_socket();
  printf("Connected to %s:%d\n", HOST, PORT);
  if (!is_rfb_server(socket_fd)) {
    printf("Not a rfb server\n");
    return 1;
  }

  int logged_in = try_login(socket_fd);
  if (!logged_in) {
    printf("Login failed\n");
    return 1; // login failed
  }
  printf("Logged in\n");

  struct File **files = get_files(socket_fd);

  free(files);
  return 0;
  init();

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyDown(KEY_ENTER))
      break;
    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();
  }

  winclose();
  return 0;
}
