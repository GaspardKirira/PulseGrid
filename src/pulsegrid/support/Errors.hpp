/**
 *
 *  @file Errors.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Shared exception types for the application.
 *
 */

#ifndef PULSEGRID_SUPPORT_ERRORS_HPP
#define PULSEGRID_SUPPORT_ERRORS_HPP

#include <stdexcept>
#include <string>
#include <utility>

namespace pulsegrid::support
{
  class PulseGridError : public std::runtime_error
  {
  public:
    explicit PulseGridError(const std::string &message)
        : std::runtime_error(message)
    {
    }

    explicit PulseGridError(std::string &&message)
        : std::runtime_error(std::move(message))
    {
    }
  };

  class ConfigError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };

  class ValidationError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };

  class NotFoundError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };

  class ConflictError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };

  class PersistenceError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };

  class CheckExecutionError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };

  class RealtimeError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };

  class BootstrapError final : public PulseGridError
  {
  public:
    using PulseGridError::PulseGridError;
  };
} // namespace pulsegrid::support

#endif // PULSEGRID_SUPPORT_ERRORS_HPP
