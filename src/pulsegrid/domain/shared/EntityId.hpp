/**
 *
 *  @file EntityId.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Strong identifier type for domain entities.
 *
 */

#ifndef PULSEGRID_DOMAIN_SHARED_ENTITY_ID_HPP
#define PULSEGRID_DOMAIN_SHARED_ENTITY_ID_HPP

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace pulsegrid::domain::shared
{
  class EntityId
  {
  public:
    EntityId() = default;

    explicit EntityId(std::string value)
        : value_(std::move(value))
    {
      validate_or_throw(value_);
    }

    [[nodiscard]] const std::string &value() const noexcept
    {
      return value_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
      return value_.empty();
    }

    [[nodiscard]] std::string str() const
    {
      return value_;
    }

    static void validate_or_throw(std::string_view value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("EntityId cannot be empty");
      }
    }

    friend bool operator==(const EntityId &lhs, const EntityId &rhs) noexcept
    {
      return lhs.value_ == rhs.value_;
    }

    friend bool operator!=(const EntityId &lhs, const EntityId &rhs) noexcept
    {
      return !(lhs == rhs);
    }

    friend bool operator<(const EntityId &lhs, const EntityId &rhs) noexcept
    {
      return lhs.value_ < rhs.value_;
    }

  private:
    std::string value_;
  };

} // namespace pulsegrid::domain::shared

#endif // PULSEGRID_DOMAIN_SHARED_ENTITY_ID_HPP
