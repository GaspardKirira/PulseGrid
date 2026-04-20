/**
 *
 *  @file MonitorController.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  HTTP controller for monitor endpoints.
 *
 */

#ifndef PULSEGRID_PRESENTATION_HTTP_MONITOR_CONTROLLER_HPP
#define PULSEGRID_PRESENTATION_HTTP_MONITOR_CONTROLLER_HPP

#include <pulsegrid/application/services/MonitorService.hpp>
#include <vix.hpp>

namespace pulsegrid::presentation::http
{
  class MonitorController
  {
  public:
    explicit MonitorController(
        pulsegrid::application::services::MonitorService &monitor_service);

    /**
     * @brief Register monitor-related HTTP routes.
     */
    void register_routes(vix::App &app) const;

  private:
    pulsegrid::application::services::MonitorService &monitor_service_;
  };

} // namespace pulsegrid::presentation::http

#endif // PULSEGRID_PRESENTATION_HTTP_MONITOR_CONTROLLER_HPP
