//Que2server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>

#define MAX_MSG_SIZE 1024

double evaluate_expression(char *expr) {
    char *token;
    double result;
    char operator;

    // Get the first token
    token = strtok(expr, " ");

    // Convert the first token to a double
    sscanf(token, "%lf", &result);

    // Get subsequent tokens
    while ((token = strtok(NULL, " ")) != NULL) {
        // Check if the token is an operator
        if (strchr("+-*/", token[0]) != NULL) {
            operator = token[0];
        } else {
            // Convert the token to a double and apply the operator
            double operand;
            sscanf(token, "%lf", &operand);
            switch (operator) {
                case '+':
                    result += operand;
                    break;
                case '-':
                    result -= operand;
                    break;
                case '*':
                    result *= operand;
                    break;
                case '/':
                    result /= operand;
                    break;
                default:
                    printf("Invalid operator\n");
                    return NAN; // Not-a-Number
            }
        }
    }

    return result;
}                

// Function to handle client connections
void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[MAX_MSG_SIZE];

    while (1) {
        // Receive expression from client
        recv(client_socket, buffer, MAX_MSG_SIZE, 0);
        printf("Expression received: %s\n", buffer);

        // Check for exit condition
        if (strcmp(buffer, "quit") == 0) {
            printf("Client disconnected\n");
            close(client_socket);
            pthread_exit(NULL);
        }

        // Evaluate expression
        double result = evaluate_expression(buffer);

        // Convert result to string
        sprintf(buffer, "%.2lf", result);

        // Send result to client
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Result sent: %s\n", buffer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    // Bind socket to address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s\n", argv[1]);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // Create a new thread to handle the client
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)&new_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread
        pthread_detach(tid);
    }

    return 0;
}
//Que2client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_MSG_SIZE 1024

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <server_hostname> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  int sock_fd;
  char buffer[MAX_MSG_SIZE];

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(argv[2]));

  if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
  {
    perror("Invalid address/ Address not supported");
    exit(EXIT_FAILURE);
  }
if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
  {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    printf("Enter arithmetic expression (or 'quit' to exit): ");
    fgets(buffer, MAX_MSG_SIZE, stdin);

    buffer[strcspn(buffer, "\n")] = 0;

    send(sock_fd, buffer, strlen(buffer), 0);

    if (strcmp(buffer, "quit") == 0)
    {
      printf("Exiting...\n");
      break;
    }

    recv(sock_fd, buffer, MAX_MSG_SIZE, 0);
    printf("Result: %s\n", buffer);
  }

  close(sock_fd);

  return 0;
}
