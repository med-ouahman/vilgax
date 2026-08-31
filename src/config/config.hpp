#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

#include "types.hpp"

namespace config {

using ip_address = std::variant<ipv4, ipv6>;

struct listen_endpoint {
   ip_address  addr;
   port_number port;
   usize       backlog;
};

struct fastcgi_config {
   ip_address  addr;
   timer       connection_timeout;
   timer       read_timeout;
};

struct location_config {
   string root;
   std::optional<fastcgi_config> fastcgi_conf;
   bool autoindex;
   vector<string> index;
   
};

struct server_config {
   vector<listen_endpoint> listens;
   string root;
   vector<location_config> locations;
   vector<string> server_names;
};

struct runtime_config {
   usize workers;
   bool  workers_auto;
   usize max_connections;
   usize max_conns_per_worker;

  string user;
  string group;
   
  string pid_file;
  string access_log;
  string error_log;

};

struct global_http_config {
   usize max_request_line_size;
   usize max_header_size;
   usize max_headers;
   usize max_requests_per_connection;
   bool  keepalive;
   bool  sendfile;
   usize sendfile_min_size;
};

struct timeout_http_config {
   timer headers;
   timer body;
   timer keepalive;
   timer write;
};

struct main_config {
   runtime_config       runtime_conf;
   global_http_config   http_conf;
   timeout_http_config  http_timeout_conf;
   vector<server_config> servers;
};

}
