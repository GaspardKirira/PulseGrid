/**
 *
 *  @file MonitorController.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "MonitorController.hpp"

#include <exception>
#include <string>

#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/presentation/dto/CreateMonitorRequest.hpp>
#include <pulsegrid/presentation/dto/MonitorResponse.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <pulsegrid/support/Json.hpp>
#include <vix/json/json.hpp>
#include <pulsegrid/infrastructure/runtime/AppLogger.hpp>

namespace pulsegrid::presentation::http
{
  namespace
  {
    /**
     * @brief Send a JSON error response.
     *
     * @param res HTTP response object.
     * @param status HTTP status code.
     * @param message Error message.
     */
    void send_error(vix::Response &res, int status, const std::string &message)
    {
      namespace J = vix::json;

      res.status(status).json(
          J::o(
              "ok", false,
              "error", message));
    }
  } // namespace

  MonitorController::MonitorController(
      pulsegrid::application::services::MonitorService &monitor_service)
      : monitor_service_(monitor_service)
  {
  }

  void MonitorController::register_routes(vix::App &app) const
  {
    app.get("/api/monitors", [this](vix::Request &, vix::Response &res)
            {
  namespace J = vix::json;

  try
  {
    const auto monitors = monitor_service_.list_monitors();

    J::Json items = J::Json::array();
    for (const auto &monitor : monitors)
    {
      items.push_back(
          pulsegrid::presentation::dto::MonitorResponse::from_domain(monitor));
    }

    res.status(200).json(
        J::o(
            "ok", true,
            "count", static_cast<long long>(monitors.size()),
            "data", items));
  }
  catch (const std::exception &e)
  {
    send_error(res, 500, e.what());
  } });

    app.get("/api/monitors/{id}", [this](vix::Request &req, vix::Response &res)
            {
              try
              {
                const auto id = pulsegrid::domain::shared::EntityId(
                    req.param("id", ""));

                const auto monitor = monitor_service_.get_monitor_by_id(id);

                res.status(200).json(
                    pulsegrid::presentation::dto::MonitorResponse::from_domain(monitor));
              }
              catch (const pulsegrid::support::NotFoundError &e)
              {
                send_error(res, 404, e.what());
              }
              catch (const std::exception &e)
              {
                send_error(res, 400, e.what());
              } });

    app.post("/api/monitors", [this](vix::Request &req, vix::Response &res)
             {
               try
               {
                 const auto body = pulsegrid::support::try_parse(req.body());
                 if (!body.has_value())
                 {
                   send_error(res, 400, "Invalid JSON body");
                   return;
                 }

                 const auto dto =
                     pulsegrid::presentation::dto::CreateMonitorRequest::from_json(*body);

                 const auto created =
                     monitor_service_.create_monitor(dto.to_create_input());

                 res.status(201).json(
                     pulsegrid::presentation::dto::MonitorResponse::from_domain(created));
               }
               catch (const pulsegrid::support::ConflictError &e)
               {
                 send_error(res, 409, e.what());
               }
               catch (const pulsegrid::support::ValidationError &e)
               {
                 send_error(res, 400, e.what());
               }
               catch (const std::exception &e)
               {
                 send_error(res, 500, e.what());
               } });

    app.put("/api/monitors/{id}", [this](vix::Request &req, vix::Response &res)
            {
              try
              {
                const auto body = pulsegrid::support::try_parse(req.body());
                if (!body.has_value())
                {
                  send_error(res, 400, "Invalid JSON body");
                  return;
                }

                const auto id = pulsegrid::domain::shared::EntityId(
                    req.param("id", ""));

                const auto dto =
                    pulsegrid::presentation::dto::CreateMonitorRequest::from_json(*body);

                const auto updated =
                    monitor_service_.update_monitor(id, dto.to_update_input());

                res.status(200).json(
                    pulsegrid::presentation::dto::MonitorResponse::from_domain(updated));
              }
              catch (const pulsegrid::support::NotFoundError &e)
              {
                send_error(res, 404, e.what());
              }
              catch (const pulsegrid::support::ConflictError &e)
              {
                send_error(res, 409, e.what());
              }
              catch (const pulsegrid::support::ValidationError &e)
              {
                send_error(res, 400, e.what());
              }
              catch (const std::exception &e)
              {
                send_error(res, 500, e.what());
              } });

    app.post("/api/monitors/{id}/pause", [this](vix::Request &req, vix::Response &res)
             {
               try
               {
                 const auto id = pulsegrid::domain::shared::EntityId(
                     req.param("id", ""));

                 const auto updated = monitor_service_.pause_monitor(id);

                 res.status(200).json(
                     pulsegrid::presentation::dto::MonitorResponse::from_domain(updated));
               }
               catch (const pulsegrid::support::NotFoundError &e)
               {
                 send_error(res, 404, e.what());
               }
               catch (const std::exception &e)
               {
                 send_error(res, 400, e.what());
               } });

    app.post("/api/monitors/{id}/resume", [this](vix::Request &req, vix::Response &res)
             {
               try
               {
                 const auto id = pulsegrid::domain::shared::EntityId(
                     req.param("id", ""));

                 const auto updated = monitor_service_.resume_monitor(id);

                 res.status(200).json(
                     pulsegrid::presentation::dto::MonitorResponse::from_domain(updated));
               }
               catch (const pulsegrid::support::NotFoundError &e)
               {
                 send_error(res, 404, e.what());
               }
               catch (const std::exception &e)
               {
                 send_error(res, 400, e.what());
               } });

    app.del("/api/monitors/{id}", [this](vix::Request &req, vix::Response &res)
            {
              namespace J = vix::json;

              try
              {
                const auto id = pulsegrid::domain::shared::EntityId(
                    req.param("id", ""));

                const bool removed = monitor_service_.delete_monitor(id);
                if (!removed)
                {
                  send_error(res, 404, "Monitor not found");
                  return;
                }

                res.status(200).json(
                    J::o(
                        "ok", true,
                        "deleted", true,
                        "id", id.value()));
              }
              catch (const std::exception &e)
              {
                send_error(res, 400, e.what());
              } });
  }

} // namespace pulsegrid::presentation::http
