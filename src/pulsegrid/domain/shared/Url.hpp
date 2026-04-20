/**
 *
 *  @file Url.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Simple validated URL value object.
 *
 */

#ifndef PULSEGRID_DOMAIN_SHARED_URL_HPP
#define PULSEGRID_DOMAIN_SHARED_URL_HPP

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace pulsegrid::domain::shared
{
  class Url
  {
  public:
    Url() = default;

    explicit Url(std::string value)
        : value_(std::move(value))
    {
      validate_or_throw(value_);
    }

    [[nodiscard]] const std::string &value() const noexcept
    {
      return value_;
    }

    [[nodiscard]] std::string str() const
    {
      return value_;
    }

    [[nodiscard]] bool empty() const noexcept
    {
      return value_.empty();
    }

    [[nodiscard]] bool is_https() const noexcept
    {
      return starts_with_ignore_case(value_, "https://");
    }

    [[nodiscard]] bool is_http() const noexcept
    {
      return starts_with_ignore_case(value_, "http://") || is_https();
    }

    static void validate_or_throw(std::string_view value)
    {
      if (value.empty())
      {
        throw std::invalid_argument("Url cannot be empty");
      }

      if (!(starts_with_ignore_case(value, "http://") ||
            starts_with_ignore_case(value, "https://")))
      {
        throw std::invalid_argument("Url must start with http:// or https://");
      }

      const auto scheme_pos = value.find("://");
      if (scheme_pos == std::string_view::npos)
      {
        throw std::invalid_argument("Url is invalid");
      }

      const std::string_view rest = value.substr(scheme_pos + 3);
      if (rest.empty())
      {
        throw std::invalid_argument("Url host cannot be empty");
      }

      if (rest.find(' ') != std::string_view::npos)
      {
        throw std::invalid_argument("Url cannot contain spaces");
      }
    }

    friend bool operator==(const Url &lhs, const Url &rhs) noexcept
    {
      return lhs.value_ == rhs.value_;
    }

    friend bool operator!=(const Url &lhs, const Url &rhs) noexcept
    {
      return !(lhs == rhs);
    }

  private:
    static bool starts_with_ignore_case(std::string_view text, std::string_view prefix) noexcept
    {
      if (text.size() < prefix.size())
      {
        return false;
      }

      return std::equal(
          prefix.begin(),
          prefix.end(),
          text.begin(),
          [](char a, char b)
          {
            return std::tolower(static_cast<unsigned char>(a)) ==
                   std::tolower(static_cast<unsigned char>(b));
          });
    }

  private:
    std::string value_;
  };

} // namespace pulsegrid::domain::shared

#endif // PULSEGRID_DOMAIN_SHARED_URL_HPP
