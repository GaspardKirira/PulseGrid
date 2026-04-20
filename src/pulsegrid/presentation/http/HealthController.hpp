/**
 *
 *  @file HealthController.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  HTTP controller for health endpoints.
 *
 */

#ifndef PULSEGRID_PRESENTATION_HTTP_HEALTH_CONTROLLER_HPP
#define PULSEGRID_PRESENTATION_HTTP_HEALTH_CONTROLLER_HPP

#include <vix.hpp>

namespace pulsegrid::presentation::http
{
  class HealthController
  {
  public:
    HealthController() = default;

    /**
     * @brief Register health-related HTTP routes.
     */
    void register_routes(vix::App &app) const;
  };

} // namespace pulsegrid::presentation::http

#endif // PULSEGRID_PRESENTATION_HTTP_HEALTH_CONTROLLER_HPP
