/**
 *
 *  @file StatusPageController.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "StatusPageController.hpp"

#include <exception>
#include <string>

#include <pulsegrid/presentation/dto/StatusSummaryResponse.hpp>
#include <pulsegrid/support/Constants.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <vix/json/json.hpp>

namespace pulsegrid::presentation::http
{
  namespace
  {
    void send_error(vix::Response &res, int status, const std::string &message)
    {
      namespace J = vix::json;

      res.status(status).json(J::o(
          "ok", false,
          "error", message));
    }
  } // namespace

  StatusPageController::StatusPageController(
      pulsegrid::application::services::StatusService &status_service)
      : status_service_(status_service)
  {
  }

  void StatusPageController::register_routes(vix::App &app) const
  {
    app.get("/api/status/summary", [this](vix::Request &, vix::Response &res)
            {
              try
              {
                const auto summary = status_service_.get_system_summary();

                res.status(200).json(
                    pulsegrid::presentation::dto::StatusSummaryResponse::from_summary(summary));
              }
              catch (const std::exception &e)
              {
                send_error(res, 500, e.what());
              } });

    app.get("/api/status/{slug}", [this](vix::Request &req, vix::Response &res)
            {
              try
              {
                const std::string slug = req.param("slug", "");
                const auto view = status_service_.get_monitor_status_by_slug(slug);

                res.status(200).json(
                    pulsegrid::presentation::dto::StatusSummaryResponse::from_view(view));
              }
              catch (const pulsegrid::support::NotFoundError &e)
              {
                send_error(res, 404, e.what());
              }
              catch (const std::exception &e)
              {
                send_error(res, 400, e.what());
              } });

    app.get("/status/{slug}", [](vix::Request &, vix::Response &res)
            { res.file("public/status.html"); });
  }

} // namespace pulsegrid::presentation::http
