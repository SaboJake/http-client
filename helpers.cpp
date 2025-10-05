#include "helpers.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <algorithm>

int open_connection(const char *host_ip, int port, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in server_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = ip_type;
    server_addr.sin_port = htons(port);
    inet_aton(host_ip, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERROR connecting");
        close(sockfd);
        return -1;
    }
    return sockfd;
}

void close_connection(int sockfd)
{
    if (sockfd >= 0) {
        close(sockfd);
    }
}

void send_message(int sockfd, const char *message)
{
    int bytes, bytes_sent = 0;
    int len = strlen(message);

    do {
        bytes = write(sockfd, message + bytes_sent, len - bytes_sent);
        if (bytes < 0) {
            perror("ERROR writing to socket");
            return;
        }
        if (bytes == 0) {
            break;
        }
        bytes_sent += bytes;
    } while (bytes_sent < len);
}

std::string receive_message(int sockfd)
{
    char buf[BUFSIZE];
    std::string response;
    int bytes;

    do {
        memset(buf, 0, BUFSIZE);
        bytes = read(sockfd, buf, BUFSIZE - 1);
        if (bytes < 0) {
            perror("ERROR reading response from socket");
            return "";
        }
        if (bytes == 0) {
            break;
        }
        response.append(buf, bytes);
        // SO tema bonus 2 vibes
    } while (strstr(response.c_str(), "\r\n\r\n") == nullptr || 
             (strstr(response.c_str(), "Content-Length: ") != nullptr && 
              response.length() < (size_t)(strstr(response.c_str(), "\r\n\r\n") - response.c_str() + 4 +
                                        atoi(strstr(response.c_str(), "Content-Length: ") + 16)))
            );

    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
        std::string header = response.substr(0, pos);
        size_t content_length_pos = header.find("Content-Length: ");
        if (content_length_pos != std::string::npos) {
            int content_length = std::stoi(header.substr(content_length_pos + 16));
            size_t body_start = pos + 4;
            size_t body_len = response.length() - body_start;

            while (body_len < (size_t)content_length) {
                memset(buf, 0, BUFSIZE);
                bytes = read(sockfd, buf, BUFSIZE - 1);
                if (bytes < 0) {
                    perror("ERROR reading response body from socket");
                    break;
                }
                if (bytes == 0) {
                    break;
                }
                response.append(buf, bytes);
                body_len += bytes;
            }
        }
    }

    return response;
}

int extract_status_code(const std::string &response)
{
    size_t pos1 = response.find(' ');
    if (pos1 == std::string::npos) {
        return -1;
    }

    size_t pos2 = response.find(' ', pos1 + 1);
    if (pos2 == std::string::npos){ 
        return -1;
    }

    try {
        return std::stoi(response.substr(pos1 + 1, pos2 - pos1 - 1));
    } catch (const std::exception &e) {
        return -1;
    }
}

std::vector<std::string> extract_cookies(const std::string &response)
{
    std::vector<std::string> cookies;
    size_t pos = 0;
    std::string header = "Set-Cookie: ";

    while ((pos = response.find(header, pos)) != std::string::npos) {
        pos += header.length();
        size_t end = response.find("\r\n", pos);
        if (end == std::string::npos) {
            break;
        }
        std::string cookie = response.substr(pos, end - pos);
        size_t semicolon_pos = cookie.find(';');
        if (semicolon_pos != std::string::npos) {
            cookies.push_back(cookie.substr(0, semicolon_pos));
        } else {
            cookies.push_back(cookie);
        }
    }
    return cookies;
}

std::string extract_json_body(const std::string &response)
{
    size_t pos = response.find("\r\n\r\n");
    if (pos == std::string::npos) {
        if (!response.empty() && (response[0] == '{' || response[0] == '[')) {
            return response;
        }
        return "";
    }
    return response.substr(pos + 4);
}

bool is_number(const std::string &s)
{
    if (s.empty()) return false;

    bool decimal_point = false;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '.') {
            if (decimal_point) {
                return false;
            }
            decimal_point = true;
            if (i == 0 && s.length() == 1) {
                return false;
            }
        } else if (!std::isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

bool input_check(const std::vector<std::string> &fields, const std::vector<std::string> &field_names,
                 const std::vector<restriction> &restrictions)
{
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].empty()) {
            print_error("Field " + field_names[i] + " cannot be empty");
            return false;
        }
        if (restrictions[i] == NO_SPACES && fields[i].find(' ') != std::string::npos) {
            print_error("Field " + field_names[i] + " cannot contain spaces");
            return false;
        } else if (restrictions[i] == NUMBER && !is_number(fields[i])) {
            print_error("Field " + field_names[i] + " must be a number");
            return false;
        } else if (restrictions[i] == NATURAL) {
            bool check = is_number(fields[i]);
            if (!check || (check && fields[i].find('.') != std::string::npos)) {
                print_error("Field " + field_names[i] + " must be a natural number");
                return false;
            }
        }
    }
    return true;
}

std::string get_user_input(const std::string &prompt)
{
    std::string input;
    std::cout << prompt << "=";
    std::getline(std::cin, input);
    return input;
}

std::string get_response(int &sockfd, const std::string &request)
{
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        print_error("Failed to connect");
        return "";
    }
    send_message(sockfd, request.c_str());
    std::string response = receive_message(sockfd);
    close_connection(sockfd);
    sockfd = -1;

    return response;
}

void print_success(const std::string &message) {
    std::cout << "SUCCESS: " << message << "\n";
}

void print_error(const std::string &message) {
    std::cout << "ERROR: " << message << "\n";
}

void print_error(const std::string &message, int status_code) {
    std::cout << "ERROR: (" << status_code << ") " << message << "\n";
}
