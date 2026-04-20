/**
 *
 *  @file Constants.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Shared application constants.
 *
 */

#ifndef PULSEGRID_SUPPORT_CONSTANTS_HPP
#define PULSEGRID_SUPPORT_CONSTANTS_HPP

#include <chrono>
#include <string_view>

namespace pulsegrid::support::constants
{
  inline constexpr std::string_view app_name = "PulseGrid";
  inline constexpr std::string_view app_version = "0.1.0";

  inline constexpr std::string_view default_bind_host = "0.0.0.0";
  inline constexpr int default_http_port = 8080;
  inline constexpr int default_ws_port = 9090;

  inline constexpr std::string_view default_database_path = "storage/pulsegrid.db";
  inline constexpr std::string_view default_public_dir = "public";
  inline constexpr std::string_view default_migrations_dir = "migrations";

  inline constexpr std::string_view api_prefix = "/api";
  inline constexpr std::string_view status_prefix = "/status";
  inline constexpr std::string_view ws_status_endpoint = "/ws/status";

  inline constexpr std::string_view default_status_slug = "default";

  inline constexpr std::chrono::seconds default_check_interval{60};
  inline constexpr std::chrono::seconds min_check_interval{15};
  inline constexpr std::chrono::seconds max_check_interval{3600};

  inline constexpr std::chrono::seconds default_request_timeout{10};
  inline constexpr std::chrono::seconds incident_open_grace_period{0};

  inline constexpr std::size_t default_recent_checks_limit = 50;
  inline constexpr std::size_t default_recent_incidents_limit = 20;

  inline constexpr std::string_view env_http_host = "SERVER_HOST";
  inline constexpr std::string_view env_http_port = "SERVER_PORT";
  inline constexpr std::string_view env_ws_port = "WEBSOCKET_PORT";
  inline constexpr std::string_view env_db_engine = "DATABASE_ENGINE";
  inline constexpr std::string_view env_db_sqlite_path = "DATABASE_SQLITE_PATH";
  inline constexpr std::string_view env_log_level = "LOG_LEVEL";
} // namespace pulsegrid::support::constants

#endif // PULSEGRID_SUPPORT_CONSTANTS_HPP
