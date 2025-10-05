#include "helpers.h"
#include "requests.h"
#include "commands.h"
#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>
#include <sys/socket.h>

using json = nlohmann::json;

std::vector<std::string> admin_sesion_cookies;
std::vector<std::string> user_sesion_cookies;
std::string jwt_token;
std::string current_admin;
std::string current_user;

int main()
{
    std::string command;

    auto command_map = init_map();
    const std::vector<std::string> admin_commands = {"add_user", "get_users", "delete_user", "logout_admin"};
    const std::vector<std::string> user_commands = {"get_movies", "get_movie", "add_movie", "delete_movie", "update_movie",
                                                    "get_collections", "get_collection", "add_collection", "delete_collection",
                                                    "add_movie_to_collection", "delete_movie_from_collection", "logout"};

    while (true) {
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }

        int sockfd = -1;

        auto make_request = [&](const std::string &req_str, bool expect_json_response = true) -> std::pair<int, json> {
            std::string response = get_response(sockfd, req_str);

            if (response.empty()) {
                print_error("No resonse from server or connection error");
                return {-1, nullptr};
            }

            int status_code = extract_status_code(response);
            std::string json_body = extract_json_body(response);

            json response_json = nullptr;
            if (expect_json_response && !json_body.empty()) {
                try {
                    if (json_body[0] == '{' || json_body[0] == '[') {
                        response_json = json::parse(json_body);
                    } else {
                        // Non json response
                        if (status_code >= 400) {
                            print_error(json_body, status_code);
                            return {status_code, nullptr};
                        }
                    }
                } catch (const json::parse_error &e) {
                    if (status_code >= 200 && status_code < 300) {
                        print_error("Expected JSON response but got: " + json_body, status_code);
                    } else {
                        print_error(json_body.empty() ? "Unknown error" : json_body, status_code);
                    }
                    return {status_code, nullptr};
                }
            } else if (expect_json_response && json_body.empty() && status_code >= 200 && status_code < 300) {
                // Empty response but success status code (no body)
            }

            if (status_code >= 400) {
                if (response_json != nullptr && response_json.contains("error")) {
                    print_error(response_json["error"].get<std::string>(), status_code);
                } else if (!response_json.empty()) {
                    std::string err_msg = json_body;
                    if (err_msg.length() > 200) {
                        err_msg = err_msg.substr(0, 200) + "...";
                    }
                    print_error(err_msg, status_code);
                } else {
                    print_error("Unknown error", status_code);
                }
            }

            return {status_code, response_json};
        };

        if (std::find(admin_commands.begin(), admin_commands.end(), command) != admin_commands.end()) {
            if (admin_sesion_cookies.empty()) {
                print_error("Admin not logged in or session expired");
                continue;
            }
        }
        if (std::find(user_commands.begin(), user_commands.end(), command) != user_commands.end()) {
            if (jwt_token.empty()) {
                print_error("No access to library. Please run 'get_access' first.");
                continue;
            }
        }

        if (command == "login_admin") {
            log_in(sockfd, false);
            continue;
        } else if (command == "login") {
            log_in(sockfd, true);
            continue;
        }

        auto it = command_map.find(command);
        if (it != command_map.end()) {
            it->second(make_request);
        } else {
            if (!command.empty()) {
                print_error("Unknown command: " + command);
            }
        }

        if (sockfd != -1) {
            close_connection(sockfd);
            sockfd = -1;
        }
    }

    return 0;
}
