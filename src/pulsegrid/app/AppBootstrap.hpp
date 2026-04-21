/**
 *
 *  @file AppBootstrap.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application bootstrap and composition root.
 *
 */

#ifndef PULSEGRID_APP_APP_BOOTSTRAP_HPP
#define PULSEGRID_APP_APP_BOOTSTRAP_HPP

#include <memory>
#include <atomic>
#include <thread>

#include <pulsegrid/app/MiddlewareRegistry.hpp>
#include <pulsegrid/app/RouteRegistry.hpp>
#include <pulsegrid/application/services/CheckService.hpp>
#include <pulsegrid/application/services/IncidentService.hpp>
#include <pulsegrid/application/services/MonitorService.hpp>
#include <pulsegrid/application/services/StatusService.hpp>
#include <pulsegrid/infrastructure/checker/CheckScheduler.hpp>
#include <pulsegrid/infrastructure/checker/HttpChecker.hpp>
#include <pulsegrid/infrastructure/db/SqliteCheckResultRepository.hpp>
#include <pulsegrid/infrastructure/db/SqliteIncidentRepository.hpp>
#include <pulsegrid/infrastructure/db/SqliteMonitorRepository.hpp>
#include <pulsegrid/infrastructure/events/WsEventBus.hpp>
#include <pulsegrid/infrastructure/runtime/AppLogger.hpp>
#include <pulsegrid/infrastructure/runtime/SystemClock.hpp>
#include <pulsegrid/infrastructure/runtime/UuidGenerator.hpp>
#include <pulsegrid/presentation/http/HealthController.hpp>
#include <pulsegrid/presentation/http/MonitorController.hpp>
#include <pulsegrid/presentation/http/StatusPageController.hpp>
#include <pulsegrid/presentation/ws/StatusWsGateway.hpp>

#include <vix.hpp>
#include <vix/config/Config.hpp>
#include <vix/db/Database.hpp>
#include <vix/websocket/App.hpp>

namespace pulsegrid::app
{
  class AppBootstrap
  {
  public:
    AppBootstrap();

    /**
     * @brief Start the HTTP application.
     *
     * This is the main entry point used by src/main.cpp.
     */
    int run();

  private:
    void initialize();
    void initialize_database();
    void initialize_repositories();
    void initialize_realtime();
    void initialize_services();
    void initialize_controllers();
    void initialize_registries();
    void initialize_http_app();
    void initialize_ws_app();
    void start_scheduler();
    void stop_scheduler();

  private:
    bool initialized_{false};

    std::unique_ptr<vix::config::Config> config_;
    std::shared_ptr<vix::db::Database> database_;

    std::unique_ptr<vix::App> http_app_;
    std::shared_ptr<vix::executor::RuntimeExecutor> ws_executor_;
    std::unique_ptr<vix::websocket::App> ws_app_;

    std::unique_ptr<pulsegrid::infrastructure::runtime::SystemClock> clock_;
    std::unique_ptr<pulsegrid::infrastructure::runtime::UuidGenerator> id_generator_;
    std::unique_ptr<pulsegrid::infrastructure::checker::HttpChecker> http_checker_;

    std::unique_ptr<pulsegrid::infrastructure::db::SqliteMonitorRepository> monitor_repository_;
    std::unique_ptr<pulsegrid::infrastructure::db::SqliteCheckResultRepository> check_result_repository_;
    std::unique_ptr<pulsegrid::infrastructure::db::SqliteIncidentRepository> incident_repository_;

    std::unique_ptr<pulsegrid::presentation::ws::StatusWsGateway> status_ws_gateway_;
    std::unique_ptr<pulsegrid::infrastructure::events::WsEventBus> event_bus_;

    std::unique_ptr<pulsegrid::application::services::MonitorService> monitor_service_;
    std::unique_ptr<pulsegrid::application::services::CheckService> check_service_;
    std::unique_ptr<pulsegrid::application::services::IncidentService> incident_service_;
    std::unique_ptr<pulsegrid::application::services::StatusService> status_service_;

    std::unique_ptr<pulsegrid::infrastructure::checker::CheckScheduler> check_scheduler_;

    std::unique_ptr<pulsegrid::presentation::http::HealthController> health_controller_;
    std::unique_ptr<pulsegrid::presentation::http::MonitorController> monitor_controller_;
    std::unique_ptr<pulsegrid::presentation::http::StatusPageController> status_page_controller_;

    std::unique_ptr<pulsegrid::app::MiddlewareRegistry> middleware_registry_;
    std::unique_ptr<pulsegrid::app::RouteRegistry> route_registry_;

    std::atomic<bool> scheduler_running_{false};
    std::thread scheduler_thread_{};
  };

} // namespace pulsegrid::app

#endif // PULSEGRID_APP_APP_BOOTSTRAP_HPP
