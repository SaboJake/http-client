#ifndef REQUESTS_H
#define REQUESTS_H

#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

std::string get_request(const std::string &host, const std::string &url,
                                const std::string &query_params,
                                const std::vector<std::string> &cookies,
                                const std::string &jwt_token);

std::string post_request(const std::string &host, const std::string &url,
                                 const std::string &content_type, const json &body_data,
                                 const std::vector<std::string> &cookies,
                                 const std::string &jwt_token);

std::string delete_request(const std::string &host, const std::string &url,
                                   const std::vector<std::string> &cookies,
                                   const std::string &jwt_token);

std::string put_request(const std::string &host, const std::string &url,
                                const std::string &content_type, const json &body_data,
                                const std::vector<std::string> &cookies,
                                const std::string &jwt_token);

#endif
