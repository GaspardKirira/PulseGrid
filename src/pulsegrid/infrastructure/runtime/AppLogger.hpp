/**
 *
 *  @file AppLogger.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Small application logging helper.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_RUNTIME_APP_LOGGER_HPP
#define PULSEGRID_INFRASTRUCTURE_RUNTIME_APP_LOGGER_HPP

#include <string>
#include <string_view>
#include <utility>

#include <vix/log/Log.hpp>

namespace pulsegrid::infrastructure::runtime
{
  class AppLogger
  {
  public:
    static void set_info_level()
    {
      vix::log::set_level(vix::log::LogLevel::Info);
    }

    static void set_debug_level()
    {
      vix::log::set_level(vix::log::LogLevel::Debug);
    }

    static void set_json_format()
    {
      vix::log::set_format(vix::log::LogFormat::JSON);
    }

    static void set_kv_format()
    {
      vix::log::set_format(vix::log::LogFormat::KV);
    }

    static void info(std::string_view message)
    {
      vix::log::info("{}", std::string(message));
    }

    static void warn(std::string_view message)
    {
      vix::log::warn("{}", std::string(message));
    }

    static void error(std::string_view message)
    {
      vix::log::error("{}", std::string(message));
    }

    template <typename... Args>
    static void infof(std::string_view message, Args &&...args)
    {
      vix::log::logf(
          vix::log::LogLevel::Info,
          std::string(message),
          std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warnf(std::string_view message, Args &&...args)
    {
      vix::log::logf(
          vix::log::LogLevel::Warn,
          std::string(message),
          std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void errorf(std::string_view message, Args &&...args)
    {
      vix::log::logf(
          vix::log::LogLevel::Error,
          std::string(message),
          std::forward<Args>(args)...);
    }
  };

} // namespace pulsegrid::infrastructure::runtime

#endif // PULSEGRID_INFRASTRUCTURE_RUNTIME_APP_LOGGER_HPP
