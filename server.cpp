/*
Server:
  wait for client
  client connects
  start thread
    loop:
      read message
      understand message
      send reply
      */

//headers to use sockeys in a Linux environment
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define SERVER_PORT 7777 //provisional port chosen

int main(){

//MAKE SOCKET
//socket(domain,type,protocl) function creates a function and returns a non-neg integer if succesful 
int fd;
if((fd = socket(AF_INET, SOCK_STREAM,0)) < 0){
  std::cerr << "Socket failed to create\n";
  return 1;
} //socket created -> fd

//MAKE ADDRESS
sockaddr_in address{}; //sockaddr _in is a struct that cointains the address family (AF_INET), port number (sin_port), and IP address (sin_addr).
address.sin_family = AF_INET;
address.sin_addr.s_addr = htonl(INADDR_ANY);
address.sin_port = htons(SERVER_PORT);

//BIND SOCKET TO ADDRESS
//bind(socket, adress, adress_length) where the address points to a sockaddr structure containing the address to be bound to the socket.
if((bind(fd, (sockaddr*)&address, sizeof(address))) < 0){
  std::cerr << "Server and socket failed to bind\n";
  close(fd);
  return 1;
} //server binded

//LISTEN FOR CONNECTION
//listen(socket, backlog) where the backlog argument limits the number of outstanding connections in the socket's listen queue.
if((listen(fd, 10)) < 0){
  std::cerr << "Failed to listen\n";
  close(fd);
  return 1;
} //server listening
std::cout << "Listening on port " << SERVER_PORT << "\n";

//CONNECT
//connect(socket, address, address_len). Extracts the first connection on the queue of pending connections, create a new socket with the same socket type protocol and address family as the specified socket, and allocate a new file descriptor for that socket.
  for(;;){
    int client_fd;
    if((client_fd = accept(fd, nullptr, nullptr )) < 0){ //I do NULL for both bc I don't care who connected
    std::cerr << "Failed to accept\n";
    continue; //continue bc one failed connection doesnt mean whole server is broken
    }
    //TODO: Hand it to thread
    //client connected
    std::cout << "Accepted a client!\n";


    close(client_fd);
  }

close(fd);//close socket

return 0;
}