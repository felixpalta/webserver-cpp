#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>

static const std::string START_PAGE_NAME = "index.html";

static const std::string METHOD_GET = "GET";
static const std::string ROOT_PATH = "/";

enum HTTP_CODES {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    NOT_IMPLEMENTED = 501,
};


std::string makeReply(HTTP_CODES code){
    std::string code_description;
    switch (code) {
    case OK:
        code_description = "200 OK";
        break;
    case BAD_REQUEST:
        code_description = "400 Bad Request";
        break;
    case NOT_FOUND:
        code_description = "404 Not Found";
        break;
    case NOT_IMPLEMENTED:
        code_description = "501 Not Implemented";
        break;
    default:
        code_description = "Impossible HTTP status code";
        break;
    }

    std::string reply = "HTTP/1.1 " + code_description + " \n"
                        "Content-Type: text/html; charset=utf-8\n"
                        "\n";

    if (code != OK){
        reply +=    "<html>"
                    "<head><title>" + code_description + "</title></head>"
                    "<body><center><h1><b>" + code_description + "</b></h1></center></body>"
                    "</html>";
    }

    return reply;
}

static const unsigned BUFSIZE = 1024;
static const unsigned QUEUESIZE = 5;

static const unsigned MIN_PORT_NUMBER = 1024;

void error(const std::string& msg)
{
    perror(msg.c_str());
    exit(1);
}

void handleConnection(int clientsocket_fd);
void sendReply(int socket_fd,const std::string& reply);

int main(int argc, char *argv[])
{

     if (argc < 2) {
         std::cerr << "ERROR, no port provided" << std::endl;
         exit(1);
     }

     int port_number = atoi(argv[1]);
     if (port_number <= MIN_PORT_NUMBER) {
         std::cerr << "This port is invaid or reserved: " << port_number << std::endl;
         exit (1);
     }

     int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

     if (socket_fd < 0) error("ERROR opening socket");


     struct sockaddr_in serv_addr;
     memset(&serv_addr,0,sizeof(serv_addr));

     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(port_number);

     if (bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding");

     if (listen(socket_fd,QUEUESIZE) < 0) error("ERROR while preparing to accept connections on socket");


     for (;;){
         int clientsocket_fd = accept(socket_fd, NULL, NULL);
         if (clientsocket_fd < 0) error("ERROR on accept");

         signal(SIGCHLD, SIG_IGN);  // to avoid creating zombies
         pid_t pid = fork();

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
    if (n < 0)
    {
        close(clientsocket_fd);
        error("ERROR reading from socket");
    }

    std::cout << "Message received: " << buffer << std::endl;
    std::string msg_string(buffer);

    std::istringstream msg_stream(msg_string);

    std::string header;
    if (std::getline(msg_stream,header)) {

        std::istringstream header_stream(header);
        std::string method;

        if (header_stream >> method && method == METHOD_GET) {
            std::cout << "Method GET received" << std::endl;
            std::string path;
            if (header_stream >> path && path == ROOT_PATH)
            {

                   std::ifstream start_page_stream(START_PAGE_NAME.c_str());
                   if (start_page_stream){
                       std::stringstream file_buffer;
                       file_buffer << start_page_stream.rdbuf();
                       sendReply(clientsocket_fd,makeReply(OK));
                       sendReply(clientsocket_fd,file_buffer.str());
                       }
                   else {
                       sendReply(clientsocket_fd,makeReply(NOT_FOUND));
                       std::cout << "Path: " << path << std::endl;
                   }

            }
            else {
                sendReply(clientsocket_fd,makeReply(NOT_FOUND));
                std::cout << "Path: " << path << std::endl;
            }
        }
        else  {
            sendReply(clientsocket_fd,makeReply(NOT_IMPLEMENTED));
            std::cout << "Method: " << method << std::endl;
        }
    }
    else sendReply(clientsocket_fd,makeReply(BAD_REQUEST));

    close(clientsocket_fd);
}

void sendReply(int socket_fd,const std::string& reply){
    int n = write(socket_fd,reply.c_str(),reply.size());
    if (n < 0) {
        close(socket_fd);
        error("ERROR writing to socket");
    }
}
