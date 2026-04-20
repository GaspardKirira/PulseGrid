/**
 *
 *  @file RouteRegistry.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Central HTTP route registration.
 *
 */

#ifndef PULSEGRID_APP_ROUTE_REGISTRY_HPP
#define PULSEGRID_APP_ROUTE_REGISTRY_HPP

#include <pulsegrid/presentation/http/HealthController.hpp>
#include <pulsegrid/presentation/http/MonitorController.hpp>
#include <pulsegrid/presentation/http/StatusPageController.hpp>
#include <vix.hpp>

namespace pulsegrid::app
{
  class RouteRegistry
  {
  public:
    RouteRegistry(
        const pulsegrid::presentation::http::HealthController &health_controller,
        const pulsegrid::presentation::http::MonitorController &monitor_controller,
        const pulsegrid::presentation::http::StatusPageController &status_page_controller);

    /**
     * @brief Register all HTTP routes.
     */
    void register_all(vix::App &app) const;

  private:
    const pulsegrid::presentation::http::HealthController &health_controller_;
    const pulsegrid::presentation::http::MonitorController &monitor_controller_;
    const pulsegrid::presentation::http::StatusPageController &status_page_controller_;
  };

} // namespace pulsegrid::app

#endif // PULSEGRID_APP_ROUTE_REGISTRY_HPP
