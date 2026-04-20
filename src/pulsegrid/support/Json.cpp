/**
 *
 *  @file Json.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Implementation of central JSON utilities.
 *
 */

#include <pulsegrid/support/Json.hpp>

namespace pulsegrid::support
{

  Json parse(std::string_view data)
  {
    return vix::json::loads(std::string(data));
  }

  std::optional<Json> try_parse(std::string_view data)
  {
    try
    {
      return vix::json::loads(std::string(data));
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  std::string stringify(const Json &j, int indent)
  {
    if (indent >= 0)
    {
      return vix::json::dumps(j, indent);
    }

    return vix::json::dumps(j);
  }

  std::optional<Json> get(const Json &j, std::string_view key)
  {
    if (!j.is_object())
    {
      return std::nullopt;
    }

    const std::string json_key{key};

    if (!j.contains(json_key))
    {
      return std::nullopt;
    }

    return j.at(json_key);
  }

  bool has(const Json &j, std::string_view key)
  {
    if (!j.is_object())
    {
      return false;
    }

    return j.contains(std::string(key));
  }

  std::string get_string(const Json &j, std::string_view key, std::string_view def)
  {
    const auto value = get(j, key);
    if (!value || !value->is_string())
    {
      return std::string(def);
    }

    return value->get<std::string>();
  }

  int get_int(const Json &j, std::string_view key, int def)
  {
    const auto value = get(j, key);
    if (!value || !value->is_number_integer())
    {
      return def;
    }

    return value->get<int>();
  }

  double get_double(const Json &j, std::string_view key, double def)
  {
    const auto value = get(j, key);
    if (!value || !value->is_number())
    {
      return def;
    }

    return value->get<double>();
  }

  bool get_bool(const Json &j, std::string_view key, bool def)
  {
    const auto value = get(j, key);
    if (!value || !value->is_boolean())
    {
      return def;
    }

    return value->get<bool>();
  }

} // namespace pulsegrid::support
