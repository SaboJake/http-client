#ifndef COMMANDS_H
#define COMMANDS_H

#include "json.hpp"
#include <string>
#include <unordered_map>
#include <functional>

using json = nlohmann::json;
using make_request_type = std::function<std::pair<int, json>(const std::string&, bool)>;

extern std::vector<std::string> admin_sesion_cookies;
extern std::vector<std::string> user_sesion_cookies;
extern std::string jwt_token;
extern std::string current_admin;
extern std::string current_user;

void log_in(int sockfd, bool admin_required);

void add_or_update_movie(make_request_type make_request, bool update);

void collection_movie(make_request_type make_request, bool add);

void add_collection(make_request_type make_request);

void add_user(make_request_type make_request);

void get_users(make_request_type make_request);

void delete_user(make_request_type make_request);

void logout_admin(make_request_type make_request);

void get_access(make_request_type make_request);

void get_movies(make_request_type make_request);

void get_movie(make_request_type make_request);

void delete_movie(make_request_type make_request);

void get_collections(make_request_type make_request);

void get_collection(make_request_type make_request);

void delete_collection(make_request_type make_request);

void logout(make_request_type make_request);

std::unordered_map<std::string, std::function<void(make_request_type)>> init_map();

#endif