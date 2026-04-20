/**
 *
 *  @file Json.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Central JSON utilities.
 *  Thin wrapper around vix::json for parsing, serialization,
 *  and safe access helpers.
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

  /**
   * @brief Parse a JSON string into a Json object.
   *
   * @param data Raw JSON input.
   * @return Parsed Json value.
   *
   * @throws std::exception If the input is not valid JSON.
   */
  Json parse(std::string_view data);

  /**
   * @brief Parse a JSON string safely.
   *
   * @param data Raw JSON input.
   * @return Parsed Json on success, std::nullopt on failure.
   */
  std::optional<Json> try_parse(std::string_view data);

  /**
   * @brief Serialize a Json value into a string.
   *
   * @param j Json value to serialize.
   * @param indent Indentation level. Use a negative value for compact output.
   * @return Serialized JSON string.
   */
  std::string stringify(const Json &j, int indent = -1);

  /**
   * @brief Get a string field from a JSON object.
   *
   * @param j Source JSON object.
   * @param key Field name.
   * @param def Default value returned if the key does not exist
   *            or is not a string.
   * @return Extracted string value or default value.
   */
  std::string get_string(const Json &j, std::string_view key,
                         std::string_view def = "");

  /**
   * @brief Get an integer field from a JSON object.
   *
   * @param j Source JSON object.
   * @param key Field name.
   * @param def Default value returned if the key does not exist
   *            or is not an integer.
   * @return Extracted integer value or default value.
   */
  int get_int(const Json &j, std::string_view key, int def = 0);

  /**
   * @brief Get a boolean field from a JSON object.
   *
   * @param j Source JSON object.
   * @param key Field name.
   * @param def Default value returned if the key does not exist
   *            or is not a boolean.
   * @return Extracted boolean value or default value.
   */
  bool get_bool(const Json &j, std::string_view key, bool def = false);

  /**
   * @brief Get a floating-point field from a JSON object.
   *
   * @param j Source JSON object.
   * @param key Field name.
   * @param def Default value returned if the key does not exist
   *            or is not numeric.
   * @return Extracted floating-point value or default value.
   */
  double get_double(const Json &j, std::string_view key, double def = 0.0);

  /**
   * @brief Get a JSON field from a JSON object.
   *
   * @param j Source JSON object.
   * @param key Field name.
   * @return Field value if present, otherwise std::nullopt.
   */
  std::optional<Json> get(const Json &j, std::string_view key);

  /**
   * @brief Check whether a key exists in a JSON object.
   *
   * @param j Source JSON object.
   * @param key Field name.
   * @return true if the key exists, false otherwise.
   */
  bool has(const Json &j, std::string_view key);

} // namespace pulsegrid::support

#endif // PULSEGRID_SUPPORT_JSON_HPP
