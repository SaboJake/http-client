#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

#define HOST "63.32.125.183"
#define PORT 8081

#define BUFSIZE 4096

enum restriction {
    NONE,
    NO_SPACES,
    NUMBER,
    NATURAL,
};

int open_connection(const char *host_ip, int port, int ip_type, int socket_type, int flag);

void close_connection(int sockfd);

void send_message(int sockfd, const char *message);

std::string receive_message(int sockfd);

int extract_status_code(const std::string &response);

std::vector<std::string> extract_cookies(const std::string &response);

std::string extract_json_body(const std::string &response);

bool is_number(const std::string &s);

bool input_check(const std::vector<std::string> &fields, const std::vector<std::string> &field_names,
                 const std::vector<restriction> &restrictions);

std::string get_user_input(const std::string &prompt);

std::string get_response(int &sockfd, const std::string &request);

void print_success(const std::string &message);

void print_error(const std::string &message);
void print_error(const std::string &message, int status_code);

#endif
