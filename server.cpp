//headers to use sockeys in a Linux environment
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <cstdint>
#include <errno.h>
#include <deque>
#include <mutex>
#include "json.hpp"

using json = nlohmann::json;
#define SERVER_PORT 7777 //provisional port chosen


struct Client{

  int fd;
  std::string id;
  bool inQueue = false;

};

std::deque<std::shared_ptr<Client>> queue;
std::mutex matchmaking_mtx;
int nextSessionID = 1;

void handle_client(std::shared_ptr<Client> client){

  std::cout << "Client thread started (fd=" << client->fd << ")\n";

  std::string pending_parsing;// inside this string all the input from user will be stored

  char buffer[1024]; //created a buffer where message is going to be stored

  while (true){

    //recv(socket, buffer*, length, flags) where message is saved in buffer and it returns the length of the message
    int bytes = recv(client->fd, buffer, sizeof(buffer), 0);
    
    if (bytes > 0){//chars received
      pending_parsing.append(buffer, bytes); //add input to unparsed input string
      if (pending_parsing.size() > 64 * 1024){break;} //client wll be kicked out if sendind too many chars without enter

      size_t nl_pos;
      while ((nl_pos = pending_parsing.find('\n')) != std::string::npos){//while \n is found extract that sentence into its own string
        
        //Move one sentence from std::string pending_parsing into std::string sentence.
        std::string sentence = pending_parsing.substr(0, nl_pos);
        if (!sentence.empty() && sentence.back() == '\r'){ sentence.pop_back();} //handle window new line cases (\r\n)
        pending_parsing.erase(0, nl_pos+1);
        std::cout << "Received the sentence: " << sentence << "\n";




        try{
          json msg = json::parse(sentence);

          std::string type = msg.value("type","");
          int seq = msg.value("seq", 0);
          json payload;
          
          if(type == "hello"){
              json reply = {
                {"type", "helloBack"},
                {"seq", seq}
              };
              std::string output = reply.dump() + "\n"; //dump is a json function that dumps the json into a string format
              send(client->fd, output.c_str(), output.size(), 0); //send(socket, *buffer, length, flags) where socket is the one who it will be sent and buffer has the message
          } 

          else if (type == "join_queue"){
            std::unique_lock<std::mutex> lock(matchmaking_mtx); //lock

//if client already queued: error
            if (client->inQueue){
              json reply = {
                {"type", "error"},
                {"seq" , seq},
                {"payload",
                  {{"code", "ALREADY_QUEUED"}}
                }
              };
              std::string output = reply.dump() + "\n";
              send(client->fd, output.c_str(), output.size(), 0);
            }

//if not yet queued: queue
              else{
                queue.emplace_back(client);
                client->inQueue = true;

                if(queue.size() < 2){ //just them in queue: send queue status
                    json reply = {
                    {"type", "queue_status"},
                    {"seq", seq},             
                    {"payload",
                      {{"queued", true}}
                    }
                    };
                  std::string output = reply.dump() + "\n";
                  send(client->fd, output.c_str(), output.size(), 0);
                }

                /*TODO: 
                If queue size >= 2:

                pop two clients

                create session id

                send match_found to BOTH*/
               }
               std::unique_lock<std::mutex> unlock(matchmaking_mtx);
        }

          else{
            json reply = {
              {"type","error"},
              {"seq", seq}
            };
            std::string output = reply.dump() + "\n";
            send(client->fd, output.c_str(), output.size(), 0);
          }
        }
        catch(const json::parse_error& e){
            std::cerr << "Invalid JSON\n";
        }
      }

    }
    else if (bytes == 0){//recv returns 0 when client is disconnected
      std::cout << "Client disconnected\n";
      break; //client discconected: break loop
    }
    else{//recv returns -1 when unsuccesful
      std::cerr << "recv() error\n";
      break;
  }
}
  close(client->fd);
}


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

    //Use client struct
    static int nextID = 1;
    auto client = std::make_shared<Client>();
    client->fd = client_fd;
    client->id = "c" + std::to_string(nextID++);

    //client connected
    std::cout << "Accepted a client!\n";
    std::thread(handle_client, client).detach();

  }

close(fd);//close socket

return 0;
}