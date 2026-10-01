#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <iostream>
#include <type_traits>
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
   port_number port;
   timer       fastcgi_timeout_connect;
   timer       fastcgi_timeout_read;
};

struct redirect_conf {
   string   location;
   usize    code;
};

struct location_config {
   string path;
   string root;
   std::optional<fastcgi_config> fastcgi_conf;
   bool autoindex;
   std::vector<string> index;
   std::vector<location_config> locations;
};

struct server_config {
   string                        root;
   std::vector<string>           server_names;
   std::vector<string>           index;
   std::vector<string>           erorr_pages;
   redirect_conf                 redirect;
   std::vector<listen_endpoint>  listens;
   std::vector<location_config>  locations;
   bool                          autoindex;
   bool                          sendfile;
   usize                         sendfile_min_size;
   timer                         keepalive;
};

struct runtime_config {
   usize workers;
   bool  workers_auto;
   usize max_connections;
   usize max_connections_per_worker;

  string user;
  string group;
   
  string pid_file;
  string access_log;
  string error_log;

  usize backlog;
  
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
   timer request_line;
   timer headers;
   timer body;
   timer keepalive;
   timer write;
};

struct main_config {
   runtime_config       runtime_conf;
   global_http_config   http_conf;
   timeout_http_config  http_timeout_conf;
   std::vector<server_config> servers;
};

namespace detail {

template <typename T>
inline void display_value(std::ostream& out, const T& value) {
   if constexpr (requires { value.count(); })
      out << value.count() << " ticks";
   else if constexpr (requires { out << value; })
      out << value;
   else
      out << "<unavailable>";
}

inline void display_ip_address(std::ostream& out, const ip_address& address) {
   std::visit([&out](const auto& value) {
      using address_type = std::decay_t<decltype(value)>;
      if constexpr (requires { out << value; })
         out << value;
      else if constexpr (std::is_same_v<address_type, ipv4>)
         out << "IPv4 address";
      else
         out << "IPv6 address";
   }, address);
}

inline void display_location(std::ostream& out, const location_config& location,
                             std::size_t depth) {
   const std::string indent(depth * 2, ' ');
   out << indent << "Location " << location.path << ":\n"
       << indent << "  Root: " << location.root << '\n'
       << indent << "  Autoindex: " << (location.autoindex ? "on" : "off") << '\n'
       << indent << "  Index files:";
   if (location.index.empty()) out << " (none)";
   for (const auto& index : location.index) out << ' ' << index;
   out << '\n';

   if (location.fastcgi_conf) {
      const auto& fastcgi = *location.fastcgi_conf;
      out << indent << "  FastCGI:\n"
          << indent << "    Address: ";
      display_ip_address(out, fastcgi.addr);
        out << ':' << fastcgi.port
           << '\n' << indent << "    Connection timeout: ";
      display_value(out, fastcgi.fastcgi_timeout_connect);
      out << '\n' << indent << "    Read timeout: ";
      display_value(out, fastcgi.fastcgi_timeout_read);
      out << '\n';
   }

   for (const auto& child : location.locations)
      display_location(out, child, depth + 1);
}

} // namespace detail

inline void display_conf(const main_config& conf, std::ostream& out = std::cout) {
   const auto& runtime = conf.runtime_conf;
   const auto& http = conf.http_conf;
   const auto& timeouts = conf.http_timeout_conf;

   out << "Configuration\n"
       << "Runtime:\n"
       << "  Workers: ";
   if (runtime.workers_auto) out << "auto";
   else out << runtime.workers;
   out << "\n  Maximum connections: " << runtime.max_connections
       << "\n  Maximum connections per worker: " << runtime.max_connections_per_worker
       << "\n  User: " << runtime.user
       << "\n  Group: " << runtime.group
       << "\n  PID file: " << runtime.pid_file
       << "\n  Access log: " << runtime.access_log
       << "\n  Error log: " << runtime.error_log
       << "\n\nHTTP:\n"
       << "  Maximum request-line size: " << http.max_request_line_size
       << "\n  Maximum header size: " << http.max_header_size
       << "\n  Maximum headers: " << http.max_headers
       << "\n  Maximum requests per connection: " << http.max_requests_per_connection
       << "\n  Keep-alive: " << (http.keepalive ? "on" : "off")
       << "\n  Sendfile: " << (http.sendfile ? "on" : "off")
       << "\n  Sendfile minimum size: " << http.sendfile_min_size
       << "\n\nHTTP timeouts:\n"
         << "  Request line: ";
      detail::display_value(out, timeouts.request_line);
      out << "\n  Headers: ";
   detail::display_value(out, timeouts.headers);
   out << "\n  Body: ";
   detail::display_value(out, timeouts.body);
   out << "\n  Keep-alive: ";
   detail::display_value(out, timeouts.keepalive);
   out << "\n  Write: ";
   detail::display_value(out, timeouts.write);
   out << "\n\nServers (" << conf.servers.size() << "):\n";

   for (std::size_t i = 0; i < conf.servers.size(); ++i) {
      const auto& server = conf.servers[i];
      out << "Server " << i + 1 << ":\n"
          << "  Root: " << server.root << "\n  Names:";
      if (server.server_names.empty()) out << " (none)";
      for (const auto& name : server.server_names) out << ' ' << name;
      out << "\n  Index files:";
      if (server.index.empty()) out << " (none)";
      for (const auto& index : server.index) out << ' ' << index;
      out << "\n  Error pages:";
      if (server.erorr_pages.empty()) out << " (none)";
      for (const auto& page : server.erorr_pages) out << ' ' << page;
      out << "\n  Autoindex: " << (server.autoindex ? "on" : "off")
          << "\n  Redirect: " << server.redirect.location
          << " (code " << server.redirect.code << ")\n  Listen addresses:";
      if (server.listens.empty()) out << " (none)";
      out << '\n';
      for (const auto& endpoint : server.listens) {
         out << "    ";
         detail::display_ip_address(out, endpoint.addr);
         out << ':' << endpoint.port << " (backlog " << endpoint.backlog << ")\n";
      }
      for (const auto& location : server.locations)
         detail::display_location(out, location, 1);
      if (i + 1 < conf.servers.size()) out << '\n';
   }
}

}
