/**
 *
 *  @file HealthController.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "HealthController.hpp"

#include <pulsegrid/support/Constants.hpp>
#include <vix/json/json.hpp>

namespace pulsegrid::presentation::http
{
  void HealthController::register_routes(vix::App &app) const
  {
    app.get("/health", [](vix::Request &, vix::Response &res)
            {
              namespace J = vix::json;

              res.status(200).json(J::o(
                  "ok", true,
                  "service", std::string(pulsegrid::support::constants::app_name),
                  "version", std::string(pulsegrid::support::constants::app_version),
                  "status", "healthy")); });
  }

} // namespace pulsegrid::presentation::http
