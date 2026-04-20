/**
 *
 *  @file IdGenerator.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application id generator port.
 *
 */

#ifndef PULSEGRID_APPLICATION_PORTS_ID_GENERATOR_HPP
#define PULSEGRID_APPLICATION_PORTS_ID_GENERATOR_HPP

#include <string>

#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::application::ports
{
  class IdGenerator
  {
  public:
    virtual ~IdGenerator() = default;

    /**
     * @brief Generate a new domain entity id.
     */
    [[nodiscard]] virtual pulsegrid::domain::shared::EntityId next() = 0;
  };

} // namespace pulsegrid::application::ports

#endif // PULSEGRID_APPLICATION_PORTS_ID_GENERATOR_HPP
