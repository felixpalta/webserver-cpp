#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <iostream>
#include <string>
#include <cstring>

static const std::string reply =
        "HTTP/1.1 200 OK\n"
        "Content-Type: text/html; charset=utf-8\n"

        "\n"
        "<html><h1><b>Hello there!</b></h1></html>"
        "\n";

static const unsigned BUFSIZE = 1024;

void error(const std::string& msg)
{
    perror(msg.c_str());
    exit(1);
}

void handleConnection(int clientsocket_fd);

int main(int argc, char *argv[])
{



     if (argc < 2) {
         std::cerr << "ERROR, no port provided" << std::endl;
         exit(1);
     }

     int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

     if (socket_fd < 0)
        error("ERROR opening socket");


     struct sockaddr_in serv_addr;
     //bzero((char *) &serv_addr, sizeof(serv_addr));
     memset(&serv_addr,0,sizeof(serv_addr));

     int port_number = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(port_number);

     if (bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");

     if (listen(socket_fd,5) < 0) error("ERROR while preparing to accept connections on socket");

     struct sockaddr_in client_addr;
     socklen_t client_addr_length = sizeof(client_addr);

     for (;;){
     int clientsocket_fd = accept(socket_fd, (struct sockaddr *) &client_addr, &client_addr_length);
     if (clientsocket_fd < 0) error("ERROR on accept");

     __pid_t pid = fork();

     if (pid < 0) error("ERROR on fork");
     if (pid == 0){
         close(socket_fd);
         handleConnection(clientsocket_fd);
         exit(0);

     }
     else close(clientsocket_fd);
     }


     close(socket_fd);
     return 0;
}

void handleConnection(int clientsocket_fd){

    char buffer[BUFSIZE];
    memset(buffer,0,BUFSIZE);

    int n = read(clientsocket_fd,buffer,BUFSIZE-1);
    if (n < 0) error("ERROR reading from socket");

    std::cout << "Message received: " << buffer << std::endl;

    n = write(clientsocket_fd,reply.c_str(),reply.size());
    if (n < 0) error("ERROR writing to socket");

    close(clientsocket_fd);
}
