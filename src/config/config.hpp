#pragma once

#include <string>
#include <vector>
#include <map>
#include "types.hpp"
#include <optional>

struct ip_address {
   std::optional<ipv6> ipv6_addr;
   std::optional<ipv4> ipv4_addr;
   port_number port;
};

struct listen_endpoint {
   ip_address addr;
   usize backlog;
};

struct fastcgi {
   ip_address addr;
   timer connection_timeout;
   timer read_timeout;
};

struct location_config {
   std::string root;

   std::optional<fastcgi> fastcgi_conf;
   bool autoindex;
   std::vector<std::string> index;
   
};


struct server_config {
   std::vector<listen_endpoint> listens;
   usize backlog;
   std::string root;
   std::vector<location_config> locations;
   std::vector<std::string> server_names;
};

struct runtime_config {
   usize worker_threads;
   bool workers_auto;
   usize max_connections;
   usize max_conns_per_worker;

   std::string user;
   std::string group;
   
   std::string pid_file;
   std::string access_log;
   std::string error_log;

};

struct global_http_config {
   usize max_request_line_size;
   usize max_header_size;
   usize max_headers;
   usize max_request_conn;
   bool keepalive;
   bool sendfile;
   usize sendfile_min_size;
};

struct timeout_http_config {
   timer headers;
   timer body;
   timer keepalive;
   timer write;
   timer connect;
};

struct config {
   runtime_config globals;

   /* http */

   global_http_config   http_conf;
   timeout_http_config  http_timeout_conf;
   
   std::vector<server_config> servers;
};

