#include "commands.h"
#include "helpers.h"
#include "requests.h"
#include <iostream>

void log_in(int sockfd, bool admin_required)
{
    std::string admin_user = "placeholder";
    if (admin_required) {
        admin_user = get_user_input("admin_username");
    }
    std::string username = get_user_input("username");
    std::string password = get_user_input("password");

    if (!input_check({admin_user, username, password}, {"admin_username", "username", "password"},
                     {NO_SPACES, NO_SPACES, NO_SPACES})) {
        return;
    }
    
    if (admin_required && (!admin_sesion_cookies.empty() || !user_sesion_cookies.empty())) {
        print_error("Already logged in. Please logout first.");
        return;
    }

    json payload = {
        {"username", username},
        {"password", password}
    };
    if (admin_required) {
        payload["admin_username"] = admin_user;
    }

    std::string url = admin_required ? "/api/v1/tema/user/login" : "/api/v1/tema/admin/login";
    std::string req = post_request(HOST, url, "application/json", payload, {}, "");

    std::string response = get_response(sockfd, req);
    if (response.empty()) {
        return;
    }
    
    int status = extract_status_code(response);
    if (status == 200) {
        if (!admin_required) {
            admin_sesion_cookies = extract_cookies(response);
            if (!admin_sesion_cookies.empty()) {
                print_success("Logged in as: " + username + " (admin)");
            } else {
                print_error("Login succesful, but no cookies received");
            }
        } else {
            user_sesion_cookies = extract_cookies(response);
            if (!user_sesion_cookies.empty()) {
                print_success("Logged in as: " + username);
                current_admin = admin_user;
                current_user = username;
                jwt_token.clear();
            } else {
                print_error("Login succesful, but no cookies received");
            }
        }
    } else {
        std::string err_body = extract_json_body(response);
        try {
            json err_json = json::parse(err_body);
            if (err_json.contains("error")) print_error(err_json["error"].get<std::string>(), status);
            else print_error("Authentication failed.", status);
        } catch (...) {
            print_error(err_body.empty() ? "Authentication failed" : err_body, status);
        }
    }
}

void add_or_update_movie(make_request_type make_request, bool update)
{
    std::string id = "0";
    if (update) {
        id = get_user_input("id");
    }
    std::string title = get_user_input("title");
    std::string year = get_user_input("year");
    std::string description = get_user_input("description");
    std::string rating = get_user_input("rating");

    if (!input_check({id, title, year, description, rating}, {"id", "title", "year", "description", "rating"},
                    {NATURAL, NONE, NATURAL, NONE, NUMBER})) {
        return;
    }

    json payload = {
        {"title", title},
        {"year", std::stoi(year)},
        {"description", description},
        {"rating", std::stof(rating)}
    };

    std::string url = "/api/v1/tema/library/movies";
    if (update) {
        url += "/" + id;
    } 
    std::string req = update ? put_request(HOST, url, "application/json", payload, {}, jwt_token) :
                               post_request(HOST, url, "application/json", payload, {}, jwt_token);
    auto [status, response_json] = make_request(req, false);

    if (status >= 200 && status < 300) {
        print_success(update ? "Updated movie: " : "Added movie: " + title + " (" + year + ")");
    }
}

void collection_movie(make_request_type make_request, bool add)
{
    std::string collection_id = get_user_input("collection_id");
    std::string movie_id = get_user_input("movie_id");
    if (!input_check({collection_id, movie_id}, {"collection_id", "movie_id"}, {NATURAL, NATURAL})) {
        return;
    }
    
    json payload = {
        {"id", std::stoi(movie_id)}
    };

    std::string url = "/api/v1/tema/library/collections/" + collection_id + "/movies";
    if (!add) {
        url += "/" + movie_id;
    }
    std::string req = add ? post_request(HOST, url, "application/json", payload, {}, jwt_token) :
                            delete_request(HOST, url, {}, jwt_token);
    auto [status, response_json] = make_request(req, false);

    if (status >= 200 && status < 300) {
        print_success(add ? "Added movie to collection" : "Deleted movie from collection");
    }
}

void add_collection(make_request_type make_request)
{
    std::string title = get_user_input("title");
    std::string num_movies = get_user_input("num_movies");

    if (!input_check({title, num_movies}, {"title", "num_movies"}, {NONE, NATURAL})) {
        return;
    }
    
    int nr = std::stoi(num_movies);
    
    std::vector<std::string> movies;
    for (int i = 0; i < nr; i++) {
        std::string movie_id_field = "movie_id[" + std::to_string(i) + "]";
        movies.push_back(get_user_input(movie_id_field));
        if (!input_check({movies[i]}, {movie_id_field}, {NATURAL})) {
            return;
        }
    }

    json payload = {
        {"title", title}
    };

    std::string req = post_request(HOST, "/api/v1/tema/library/collections", "application/json", payload, {}, jwt_token);
    auto [status, response_json] = make_request(req, true);

    if (response_json == nullptr || !response_json.contains("id")) {
        print_error("Did not receive collection ID");
        return;
    }

    if (status >= 200 && status < 300) {
        int collection_id = response_json["id"].get<int>();

        for (const auto &movie_id : movies) {
            json movie_payload = {
                {"id", std::stoi(movie_id)}
            };
            std::string url = "/api/v1/tema/library/collections/" + std::to_string(collection_id) + "/movies";
            std::string req = post_request(HOST, url, "application/json", movie_payload, {}, jwt_token);
            auto [status, response_json] = make_request(req, false);
            if (status >= 200 && status < 300) {
                // Movie added successfully
            } else {
                // Error adding movie
                break;
            }
        }
        print_success("Added collection: " + title);
    }
}

void add_user(make_request_type make_request)
{
    std::string username = get_user_input("username");
    std::string password = get_user_input("password");
    if (!input_check({username, password}, {"username", "password"}, {NO_SPACES, NO_SPACES})) {
        return;
    }

    json payload = {
        {"username", username},
        {"password", password}
    };

    std::string req = post_request(HOST, "/api/v1/tema/admin/users", "application/json", payload, admin_sesion_cookies, "");
    auto [status, response_json] = make_request(req, false);
    if (status >= 200 && status < 300) {
        print_success("Added user: " + username);
    }
}

void get_users(make_request_type make_request)
{
    std::string req = get_request(HOST, "/api/v1/tema/admin/users", "", admin_sesion_cookies, "");
    auto [status, response_json] = make_request(req, true);
    
    if (status == 200 && response_json != nullptr && response_json.contains("users") && response_json["users"].is_array()) {
        print_success("User list");
        int cnt = 1;
        for (const auto &user : response_json["users"]) {
            std::cout << "#" << cnt++ << " "
                        << user["username"].get<std::string>() << ":"
                        << user["password"].get<std::string>() << "\n";
        }
    } else if (status == 200 && (response_json == nullptr || !response_json.contains("users"))) {
        print_error("Received OK status but user data is missing or malformed");
    }
}

void delete_user(make_request_type make_request)
{
    std::string username = get_user_input("username");
    if (!input_check({username}, {"username"}, {NONE})) {
        return;
    }

    std::string req = delete_request(HOST, "/api/v1/tema/admin/users/" + username, admin_sesion_cookies, "");
    auto [status, response_json] = make_request(req, false);

    if (status >= 200 && status < 300) {
        print_success("Deleted user: " + username);
    }
}

void logout_admin(make_request_type make_request)
{
    std::string req = get_request(HOST, "/api/v1/tema/admin/logout", "", admin_sesion_cookies, "");
    auto [status, response_json] = make_request(req, false);

    if (status >= 200 && status < 300) {
        admin_sesion_cookies.clear();
        print_success("Logged out (admin)");
    }
}

void get_access(make_request_type make_request)
{
    if (user_sesion_cookies.empty()) {
        print_error("User not logged in or session expired. Please login first");
        return;
    }

    std::string req = get_request(HOST, "/api/v1/tema/library/access", "", user_sesion_cookies, "");
    auto [status, response_json] = make_request(req, true);

    if (status == 200 && response_json != nullptr && response_json.contains("token")) {
        jwt_token = response_json["token"].get<std::string>();
        print_success("Received JWT token");
    } else if (status == 200 && (response_json == nullptr || !response_json.contains("token"))) {
        print_error("Access granted but no token received.");
    }
}

void get_movies(make_request_type make_request)
{
    std::string req = get_request(HOST, "/api/v1/tema/library/movies", "", user_sesion_cookies, jwt_token);
    auto [status, response_json] = make_request(req, true);

    if (status == 200 && response_json != nullptr && response_json.contains("movies") && response_json["movies"].is_array()) {
        print_success("Movie list");
        for (const auto &movie : response_json["movies"]) {
            std::cout << "#" << movie.value("id", 0) << " " << movie.value("title", "N/A") << "\n";
        }
    } else if (status == 200 && (response_json == nullptr || !response_json.contains("movies") || !response_json["movies"].is_array())) {
        print_error("Received OK status but movie data is missing or malformed");
    }    
}

void get_movie(make_request_type make_request)
{
    std::string id = get_user_input("id");
    if (!input_check({id}, {"id"}, {NATURAL})) {
        return;
    }

    std::string req = get_request(HOST, "/api/v1/tema/library/movies/" + id, "", {}, jwt_token);
    auto [status, response_json] = make_request(req, true);

    if (status == 200 && response_json != nullptr) {
        print_success("Movie details");
        std::cout << "title: " << response_json.value("title", "N/A") << "\n";
        std::cout << "year: " << response_json.value("year", 0) << "\n";
        std::cout << "description: " << response_json.value("description", "N/A") << "\n";
        std::cout << "rating: " << response_json["rating"].get<std::string>() << "\n";
    }         
}

void delete_movie(make_request_type make_request)
{
    std::string id = get_user_input("id");
    if (!input_check({id}, {"id"}, {NATURAL})) {
        return;
    }

    std::string req = delete_request(HOST, "/api/v1/tema/library/movies/" + id, {}, jwt_token);
    auto [status, response_json] = make_request(req, false);

    if (status >= 200 && status < 300) {
        print_success("Deleted movie");
    }
}

void get_collections(make_request_type make_request)
{
    std::string req = get_request(HOST, "/api/v1/tema/library/collections", "", user_sesion_cookies, jwt_token);
    auto [status, response_json] = make_request(req, true);

    if (status == 200 && response_json != nullptr && response_json.contains("collections") && response_json["collections"].is_array()) {
        print_success("Collection list");
        for (const auto &collection : response_json["collections"]) {
            std::cout << "#" << collection.value("id", 0) << " " << collection.value("title", "N/A")
                        << "; owner: " << collection.value("owner", "N/A") << "\n";
        }
    } else if (status == 200 && (response_json == nullptr || !response_json.is_array())) {
        print_error("Received OK status but collection data is missing or malformed");
    }
}

void get_collection(make_request_type make_request)
{
    std::string id = get_user_input("id");
    if (!input_check({id}, {"id"}, {NATURAL})) {
        return;
    }

    std::string req = get_request(HOST, "/api/v1/tema/library/collections/" + id, "", {}, jwt_token);
    auto [status, response_json] = make_request(req, true);

    if (status == 200 && response_json != nullptr) {
        print_success("Collection details");
        std::cout << "title: " << response_json.value("title", "N/A") << "\n";
        std::cout << "owner: " << response_json.value("owner", "N/A") << "\n";
        if (response_json.contains("movies") && response_json["movies"].is_array()) {
            std::cout << "Movies in collection:" << "\n";
            for (const auto &movie : response_json["movies"]) {
                std::cout << "#" << movie.value("id", 0) << " " << movie.value("title", "N/A") << "\n";
            }
        } else {
            print_error("Movies data is missing or malformed");
        }
    }
}

void delete_collection(make_request_type make_request)
{
    std::string id = get_user_input("id");
    if (!input_check({id}, {"id"}, {NATURAL})) {
        return;
    }

    std::string req = delete_request(HOST, "/api/v1/tema/library/collections/" + id, {}, jwt_token);
    auto [status, response_json] = make_request(req, false);

    if (status >= 200 && status < 300) {
        print_success("Deleted collection");
    }
}

void logout(make_request_type make_request)
{
    if (user_sesion_cookies.empty()) {
        print_error("User not logged in or session expired (no session cookie).");
        jwt_token.clear(); current_admin.clear(); current_user.clear();
        return;
    }
        
    std::string req = get_request(HOST, "/api/v1/tema/user/logout", "", user_sesion_cookies, "");
    auto [status, response_json] = make_request(req, false);

    if (status >= 200 && status < 300) {
        print_success("Logged out");
    }

    user_sesion_cookies.clear(); jwt_token.clear(); current_user.clear(); current_admin.clear();
}

std::unordered_map<std::string, std::function<void(make_request_type)>> init_map()
{
    std::unordered_map<std::string, std::function<void(make_request_type)>> command_map;

    command_map["add_user"] = add_user;
    command_map["get_users"] = get_users;
    command_map["delete_user"] = delete_user;
    command_map["logout_admin"] = logout_admin;
    command_map["get_access"] = get_access;
    command_map["get_movies"] = get_movies;
    command_map["get_movie"] = get_movie;
    command_map["add_movie"] = [](make_request_type make_request) {
        add_or_update_movie(make_request, false);
    };
    command_map["delete_movie"] = delete_movie;
    command_map["update_movie"] = [](make_request_type make_request) {
        add_or_update_movie(make_request, true);
    };
    command_map["get_collections"] = get_collections;
    command_map["get_collection"] = get_collection;
    command_map["add_collection"] = add_collection;
    command_map["delete_collection"] = delete_collection;
    command_map["add_movie_to_collection"] = [](make_request_type make_request) {
        collection_movie(make_request, true);
    };
    command_map["delete_movie_from_collection"] = [](make_request_type make_request) {
        collection_movie(make_request, false);
    };
    command_map["logout"] = logout;

    return command_map;
}
