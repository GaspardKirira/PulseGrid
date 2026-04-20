/**
 *
 *  @file Json.cpp
 *  PulseGrid
 *
 */

#include "Json.hpp"

#include <vix/json/json.hpp>

namespace pulsegrid::support
{
  using namespace vix::json;

  // Parsing / Serialization
  Json parse(std::string_view data)
  {
    return loads(data);
  }

  std::optional<Json> try_parse(std::string_view data)
  {
    try
    {
      return loads(data);
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  std::string stringify(const Json &j, int indent)
  {
    if (indent >= 0)
      return dumps(j, indent);

    return dumps(j);
  }

  // Safe access
  std::optional<Json> get(const Json &j, std::string_view key)
  {
    if (!j.is_object())
      return std::nullopt;

    if (!j.contains(std::string(key)))
      return std::nullopt;

    return j.at(std::string(key));
  }

  bool has(const Json &j, std::string_view key)
  {
    return j.is_object() && j.contains(std::string(key));
  }

  std::string get_string(const Json &j, std::string_view key, std::string_view def)
  {
    auto v = get(j, key);
    if (!v || !v->is_string())
      return std::string(def);

    return v->get<std::string>();
  }

  int get_int(const Json &j, std::string_view key, int def)
  {
    auto v = get(j, key);
    if (!v || !v->is_number_integer())
      return def;

    return v->get<int>();
  }

  double get_double(const Json &j, std::string_view key, double def)
  {
    auto v = get(j, key);
    if (!v || !v->is_number())
      return def;

    return v->get<double>();
  }

  bool get_bool(const Json &j, std::string_view key, bool def)
  {
    auto v = get(j, key);
    if (!v || !v->is_boolean())
      return def;

    return v->get<bool>();
  }

} // namespace pulsegrid::support
