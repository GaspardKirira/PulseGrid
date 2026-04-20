/**
 *
 *  @file Json.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Central JSON utilities (wrapper around vix::json / nlohmann::json)
 *
 */

#ifndef PULSEGRID_SUPPORT_JSON_HPP
#define PULSEGRID_SUPPORT_JSON_HPP

#include <optional>
#include <string>
#include <string_view>

#include <vix/json/json.hpp>

namespace pulsegrid::support
{
  using Json = vix::json::Json;

  // Parsing / Serialization

  /**
   * @brief Parse JSON string → Json
   * @throws std::exception if invalid
   */
  Json parse(std::string_view data);

  /**
   * @brief Safe parse (returns nullopt on failure)
   */
  std::optional<Json> try_parse(std::string_view data);

  /**
   * @brief Convert Json → string
   */
  std::string stringify(const Json &j, int indent = -1);

  // Safe access helpers
  std::string get_string(const Json &j, std::string_view key, std::string_view def = "");
  int get_int(const Json &j, std::string_view key, int def = 0);
  bool get_bool(const Json &j, std::string_view key, bool def = false);
  double get_double(const Json &j, std::string_view key, double def = 0.0);

  /**
   * @brief Get optional value
   */
  std::optional<Json> get(const Json &j, std::string_view key);

  /**
   * @brief Check if key exists
   */
  bool has(const Json &j, std::string_view key);

} // namespace pulsegrid::support

#endif // PULSEGRID_SUPPORT_JSON_HPP
