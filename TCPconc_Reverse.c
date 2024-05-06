//Que1server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUFFER_SIZE 1024

// Function to reverse a string
void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

// Function to handle client connections
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        // Null terminate the received data
        buffer[bytes_received] = '\0';

        // Reverse the received string
        reverse_string(buffer);

        // Send the reversed string back to the client
        send(client_socket, buffer, strlen(buffer), 0);
    }

    // Close the client socket
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept a new connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        printf("Connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Fork a new process to handle the client
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            // Close the server socket in the child process
            close(server_socket);

            // Handle the client
            handle_client(client_socket);

            // Close the client socket in the child process
            close(client_socket);

            // Exit the child process
            exit(EXIT_SUCCESS);
        } else { // Parent process
            // Close the client socket in the parent process
            close(client_socket);
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}




//Que1client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 9999
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    
    // Create a TCP socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Input a string to send to the server
    printf("Enter a string to reverse: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = 0; // Remove newline character from the input

    // Send the string to the server
    send(client_socket, buffer, strlen(buffer), 0);

    // Receive the reversed string from the server
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("Receive failed");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';

    printf("Reversed string received from server: %s\n", buffer);

    // Close the socket
    close(client_socket);

    return 0;
}