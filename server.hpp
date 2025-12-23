#ifndef SERVER_HPP
#define SERVER_HPP

#include <memory>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

struct Client{
  int fd;
  std::string id;
  int join_seq = 0;
  bool inQueue = false;
};

bool send_JSON_line(std::shared_ptr<Client> client, const json& j);
void handle_join_queue(std::shared_ptr<Client> client, int seq);
void handle_message(std::shared_ptr<Client> client, const json& msg);
void handle_client(std::shared_ptr<Client> client);

#endif