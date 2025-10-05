#include "requests.h"
#include <sstream>
#include <iostream>

// Cookies, JWT token, "Connection" header
void common_part(std::ostringstream &request,
                 const std::vector<std::string> &cookies,
                 const std::string &jwt_token)
{
    if (!cookies.empty()) {
        request << "Cookie: ";
        for (size_t i = 0; i < cookies.size(); i++) {
            request << cookies[i] << (i == cookies.size() - 1 ? "" : "; ");
        }
        request << "\r\n";
    }

    if (!jwt_token.empty()) {
        request << "Authorization: Bearer " << jwt_token << "\r\n";
    }
    
    request << "Connection: keep-alive\r\n";
    request << "\r\n";
}

std::string get_request(const std::string &host, const std::string &url,
                                const std::string &query_params,
                                const std::vector<std::string> &cookies,
                                const std::string &jwt_token)
{
    std::ostringstream request;
    request << "GET " << url;
    if (!query_params.empty()) {
        request << "?" << query_params;
    }
    request << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";

    common_part(request, cookies, jwt_token);

    return request.str();
}

std::string post_request(const std::string &host, const std::string &url,
                                 const std::string &content_type, const json &body_data,
                                 const std::vector<std::string> &cookies,
                                 const std::string &jwt_token) {
    std::ostringstream request;
    std::string body_str = body_data.dump();

    request << "POST " << url << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Content-Type: " << content_type << "\r\n";
    request << "Content-Length: " << body_str.length() << "\r\n";

    common_part(request, cookies, jwt_token);
    
    request << body_str;

    return request.str();
}

std::string delete_request(const std::string &host, const std::string &url,
                                   const std::vector<std::string> &cookies,
                                   const std::string &jwt_token)
{
    std::ostringstream request;
    request << "DELETE " << url << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";

    common_part(request, cookies, jwt_token);

    return request.str();
}


std::string put_request(const std::string &host, const std::string &url,
                                const std::string &content_type, const json &body_data,
                                const std::vector<std::string> &cookies,
                                const std::string &jwt_token)
{
    std::ostringstream request;
    std::string body_str = body_data.dump();

    request << "PUT " << url << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Content-Type: " << content_type << "\r\n";
    request << "Content-Length: " << body_str.length() << "\r\n";

    common_part(request, cookies, jwt_token);

    request << body_str;

    return request.str();
}
