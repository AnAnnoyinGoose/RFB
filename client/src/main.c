#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#include <raylib.h>
#include <assert.h>

#define PORT 344
#define HOST "0.0.0.0"
#define MSG "RFBv0.1nR"
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
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
  server_addr.sin_port = htons(PORT);
  (void) connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  assert(socket_fd != -1 && "connect() failed");
  return socket_fd;

}




int main(void) {
  char buffer[1024];
  int socket_fd = init_client_socket();
  ssize_t n = send(socket_fd, MSG, strlen(MSG), 0);
  assert(n > 0 && "send() failed");

  n = recv(socket_fd, buffer, 1024, 0);
  assert(n > 0 && "recv() failed");
  printf("Recieve: %s\n", buffer);
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
