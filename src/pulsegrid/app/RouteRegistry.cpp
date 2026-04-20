/**
 *
 *  @file RouteRegistry.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "RouteRegistry.hpp"

#include <string>

#include <pulsegrid/support/Constants.hpp>
#include <vix/json/json.hpp>

namespace pulsegrid::app
{
  RouteRegistry::RouteRegistry(
      const pulsegrid::presentation::http::HealthController &health_controller,
      const pulsegrid::presentation::http::MonitorController &monitor_controller,
      const pulsegrid::presentation::http::StatusPageController &status_page_controller)
      : health_controller_(health_controller),
        monitor_controller_(monitor_controller),
        status_page_controller_(status_page_controller)
  {
  }

  void RouteRegistry::register_all(vix::App &app) const
  {
    namespace J = vix::json;

    app.get("/", [](vix::Request &, vix::Response &res)
            { res.file("public/index.html"); });

    app.get("/_meta", [](vix::Request &, vix::Response &res)
            { res.status(200).json(J::o(
                  "ok", true,
                  "app", std::string(pulsegrid::support::constants::app_name),
                  "version", std::string(pulsegrid::support::constants::app_version))); });

    health_controller_.register_routes(app);
    monitor_controller_.register_routes(app);
    status_page_controller_.register_routes(app);
  }

} // namespace pulsegrid::app
