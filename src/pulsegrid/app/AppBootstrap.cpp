/**
 *
 *  @file AppBootstrap.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "AppBootstrap.hpp"

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <atomic>
#include <chrono>
#include <thread>

#include <pulsegrid/infrastructure/db/DatabaseFactory.hpp>
#include <pulsegrid/support/Constants.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <vix/websocket/AttachedRuntime.hpp>

namespace pulsegrid::app
{
  AppBootstrap::AppBootstrap() = default;

  int AppBootstrap::run()
  {
    try
    {
      initialize();

      pulsegrid::infrastructure::runtime::AppLogger::set_info_level();
      pulsegrid::infrastructure::runtime::AppLogger::set_json_format();

      pulsegrid::infrastructure::runtime::AppLogger::infof(
          "starting PulseGrid",
          "app", std::string(pulsegrid::support::constants::app_name),
          "version", std::string(pulsegrid::support::constants::app_version));

      const int port = config_->getInt(
          std::string(pulsegrid::support::constants::env_http_port),
          pulsegrid::support::constants::default_http_port);

      pulsegrid::infrastructure::runtime::AppLogger::infof(
          "http server listening",
          "port", port);

      if (!http_app_)
      {
        throw std::runtime_error("HTTP app is not initialized");
      }

      if (!ws_app_)
      {
        throw std::runtime_error("WebSocket app is not initialized");
      }

      if (!ws_executor_)
      {
        throw std::runtime_error("WebSocket executor is not initialized");
      }

      std::thread scheduler_thread([this]()
                                   {
      while (true)
      {
        try
        {
          const auto result = check_scheduler_->tick();

          pulsegrid::infrastructure::runtime::AppLogger::infof(
              "scheduler tick completed",
              "processed", static_cast<long long>(result.processed),
              "skipped", static_cast<long long>(result.skipped),
              "succeeded", static_cast<long long>(result.succeeded),
              "degraded", static_cast<long long>(result.degraded),
              "failed", static_cast<long long>(result.failed));
        }
        catch (const std::exception &e)
        {
          pulsegrid::infrastructure::runtime::AppLogger::errorf(
              "scheduler tick failed",
              "error", std::string(e.what()));
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
      } });

      scheduler_thread.detach();

      vix::run_http_and_ws(
          *http_app_,
          ws_app_->server(),
          ws_executor_,
          port);

      return 0;
    }
    catch (const std::exception &e)
    {
      pulsegrid::infrastructure::runtime::AppLogger::errorf(
          "bootstrap failed",
          "error", std::string(e.what()));
      return 1;
    }
  }

  void AppBootstrap::initialize()
  {
    if (initialized_)
    {
      return;
    }

    config_ = std::make_unique<vix::config::Config>(".env");
    clock_ = std::make_unique<pulsegrid::infrastructure::runtime::SystemClock>();
    id_generator_ = std::make_unique<pulsegrid::infrastructure::runtime::UuidGenerator>();
    http_checker_ = std::make_unique<pulsegrid::infrastructure::checker::HttpChecker>();

    initialize_database();
    initialize_repositories();
    initialize_realtime();
    initialize_services();
    initialize_controllers();
    initialize_registries();
    initialize_http_app();
    initialize_ws_app();

    initialized_ = true;
  }

  void AppBootstrap::initialize_database()
  {
    database_ = pulsegrid::infrastructure::db::DatabaseFactory::make(*config_);
  }

  void AppBootstrap::initialize_repositories()
  {
    monitor_repository_ =
        std::make_unique<pulsegrid::infrastructure::db::SqliteMonitorRepository>(database_);

    check_result_repository_ =
        std::make_unique<pulsegrid::infrastructure::db::SqliteCheckResultRepository>(database_);

    incident_repository_ =
        std::make_unique<pulsegrid::infrastructure::db::SqliteIncidentRepository>(database_);
  }

  void AppBootstrap::initialize_realtime()
  {
    ws_executor_ = std::make_shared<vix::executor::RuntimeExecutor>();

    ws_app_ = std::make_unique<vix::websocket::App>(
        ".env",
        ws_executor_);

    status_ws_gateway_ =
        std::make_unique<pulsegrid::presentation::ws::StatusWsGateway>(*ws_app_);

    event_bus_ =
        std::make_unique<pulsegrid::infrastructure::events::WsEventBus>(
            [this](const std::string &payload)
            {
              if (status_ws_gateway_)
              {
                status_ws_gateway_->broadcast(payload);
              }
            });
  }

  void AppBootstrap::initialize_services()
  {
    monitor_service_ =
        std::make_unique<pulsegrid::application::services::MonitorService>(
            *monitor_repository_,
            *clock_,
            *id_generator_,
            *event_bus_);

    check_service_ =
        std::make_unique<pulsegrid::application::services::CheckService>(
            *monitor_repository_,
            *check_result_repository_,
            *incident_repository_,
            *clock_,
            *id_generator_,
            *event_bus_);

    incident_service_ =
        std::make_unique<pulsegrid::application::services::IncidentService>(
            *incident_repository_,
            *clock_,
            *event_bus_);

    status_service_ =
        std::make_unique<pulsegrid::application::services::StatusService>(
            *monitor_repository_,
            *check_result_repository_,
            *incident_repository_);

    check_scheduler_ =
        std::make_unique<pulsegrid::infrastructure::checker::CheckScheduler>(
            *monitor_repository_,
            *check_service_,
            *clock_,
            *http_checker_);
  }

  void AppBootstrap::initialize_controllers()
  {
    health_controller_ =
        std::make_unique<pulsegrid::presentation::http::HealthController>();

    monitor_controller_ =
        std::make_unique<pulsegrid::presentation::http::MonitorController>(
            *monitor_service_);

    status_page_controller_ =
        std::make_unique<pulsegrid::presentation::http::StatusPageController>(
            *status_service_);
  }

  void AppBootstrap::initialize_registries()
  {
    middleware_registry_ =
        std::make_unique<pulsegrid::app::MiddlewareRegistry>();

    route_registry_ =
        std::make_unique<pulsegrid::app::RouteRegistry>(
            *health_controller_,
            *monitor_controller_,
            *status_page_controller_);
  }

  void AppBootstrap::initialize_http_app()
  {
    http_app_ = std::make_unique<vix::App>();

    http_app_->static_dir(
        std::string(pulsegrid::support::constants::default_public_dir));

    middleware_registry_->register_all(*http_app_);
    route_registry_->register_all(*http_app_);

    auto r = http_app_->router();
    if (r)
    {
      for (const auto &route : r->routes())
      {
        pulsegrid::infrastructure::runtime::AppLogger::infof(
            "registered route",
            "method", route.method,
            "path", route.path);
      }
    }
  }

  void AppBootstrap::initialize_ws_app()
  {
    if (!status_ws_gateway_)
    {
      throw std::runtime_error("Status WS gateway is not initialized");
    }

    status_ws_gateway_->register_routes();

    pulsegrid::infrastructure::runtime::AppLogger::info("websocket routes registered");
  }

} // namespace pulsegrid::app
