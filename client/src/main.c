#include "main.h"
#include <raylib.h>

#define PORT 344
#define HOST "0.0.0.0"

#define HEIGHT 450
#define WIDTH 800

static int socket_fd;
static Font font;

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
    // char pswd[1024];
    // printf("Enter password: ");
    // fgets(pswd, 1024, stdin);
    send(socket_fd, "password\n", strlen("password\n"), 0);
    n = recv(socket_fd, buffer, sizeof(buffer), 0);
    buffer[n] = '\0';
    if (strstr(buffer, "RFB-L-OK") != NULL)
      return 1; // login succeeded
    return 0;
  }
  return 0;
}

void init_window(void) {
  InitWindow(WIDTH, HEIGHT, "RFB Client");
  SetTargetFPS(60);
  font = LoadFontEx("resources/FiraCodeNerdFont-Regular.ttf", 18, 0, 0);
}

void make_text_button(int x, int y, int w, int h, const char *text,
                      void(callback)(void *), void *data) {
  Rectangle button = {x, y, w, h};

  if (CheckCollisionPointRec(GetMousePosition(), button)) {
    DrawRectangle(button.x, button.y, button.width, button.height, BLUE);
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      callback(data);
    }
  } else {
    DrawRectangle(button.x, button.y, button.width, button.height, GRAY);
  }
  Vector2 pos = {x + 10, y + 10};
  DrawTextEx(font, text, pos, 18, 1, BLACK);
}






void load_file_preview_window(void *data) {
  File file = *(File *)data;
  while (WindowShouldClose() == 0) {
    ClearBackground(GRAY);
    Vector2 pos = {0, 0};
    DrawTextEx(font, file.path, pos, 18, 1, BLACK);
    pos.y += 20;
    DrawTextEx(font, file.size, pos, 18, 1, BLACK);
    pos.y += 20;
    DrawTextEx(font, file.type, pos, 18, 1, BLACK);
    EndDrawing();
  }
}

void draw_file(File file, Vector2 *last_position) {
  Vector2 position = {last_position->x, last_position->y + 35};
  {
    void *data = &file;
    make_text_button(position.x, position.y, WIDTH, 30, file.name,
                     load_file_preview_window, data);
  }
  *last_position = position;
}

int main(void) {
  socket_fd = init_client_socket();
  set_file_socket_fd(socket_fd);
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

  Files files;
  init_files_vector(&files);
  get_files_from_server(&files);

  init_window();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(GRAY);
    Vector2 last_position = {0, -20};
    for (int i = 0; i < files.num_files; i++) {
      File file = files.files[i];
      set_file_name(&file);
      draw_file(file, &last_position);
    }
    EndDrawing();
  }

  return 0;
}
