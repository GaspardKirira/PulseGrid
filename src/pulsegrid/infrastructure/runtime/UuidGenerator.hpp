/**
 *
 *  @file UuidGenerator.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Simple random id generator.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_RUNTIME_UUID_GENERATOR_HPP
#define PULSEGRID_INFRASTRUCTURE_RUNTIME_UUID_GENERATOR_HPP

#include <array>
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>

#include <pulsegrid/application/ports/IdGenerator.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::infrastructure::runtime
{
  class UuidGenerator final : public pulsegrid::application::ports::IdGenerator
  {
  public:
    UuidGenerator()
        : rng_(std::random_device{}()),
          dist_(0, 255)
    {
    }

    [[nodiscard]] pulsegrid::domain::shared::EntityId next() override
    {
      return pulsegrid::domain::shared::EntityId(generate_uuid_v4());
    }

  private:
    [[nodiscard]] std::string generate_uuid_v4()
    {
      std::array<std::uint8_t, 16> bytes{};

      for (auto &b : bytes)
      {
        b = static_cast<std::uint8_t>(dist_(rng_));
      }

      // UUID v4
      bytes[6] = static_cast<std::uint8_t>((bytes[6] & 0x0F) | 0x40);
      bytes[8] = static_cast<std::uint8_t>((bytes[8] & 0x3F) | 0x80);

      std::ostringstream oss;
      oss << std::hex << std::setfill('0');

      for (std::size_t i = 0; i < bytes.size(); ++i)
      {
        oss << std::setw(2) << static_cast<int>(bytes[i]);

        if (i == 3 || i == 5 || i == 7 || i == 9)
        {
          oss << '-';
        }
      }

      return oss.str();
    }

  private:
    std::mt19937 rng_;
    std::uniform_int_distribution<int> dist_;
  };

} // namespace pulsegrid::infrastructure::runtime

#endif // PULSEGRID_INFRASTRUCTURE_RUNTIME_UUID_GENERATOR_HPP
