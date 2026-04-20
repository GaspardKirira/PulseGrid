/**
 *
 *  @file status_service_tests.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include <vix/tests/tests.hpp>

#include <optional>
#include <string>
#include <vector>

#include <pulsegrid/application/ports/CheckResultRepository.hpp>
#include <pulsegrid/application/ports/IncidentRepository.hpp>
#include <pulsegrid/application/ports/MonitorRepository.hpp>
#include <pulsegrid/application/services/StatusService.hpp>
#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/check/ResponseTime.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/monitor/CheckInterval.hpp>
#include <pulsegrid/domain/monitor/Monitor.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/domain/shared/Url.hpp>

namespace
{
  class FakeMonitorRepository final : public pulsegrid::application::ports::MonitorRepository
  {
  public:
    using Monitor = pulsegrid::domain::monitor::Monitor;
    using EntityId = pulsegrid::domain::shared::EntityId;

    void create(const Monitor &monitor) override
    {
      monitors.push_back(monitor);
    }

    void update(const Monitor &monitor) override
    {
      for (auto &item : monitors)
      {
        if (item.id() == monitor.id())
        {
          item = monitor;
          return;
        }
      }

      monitors.push_back(monitor);
    }

    [[nodiscard]] std::optional<Monitor> find_by_id(
        const EntityId &id) const override
    {
      for (const auto &item : monitors)
      {
        if (item.id() == id)
        {
          return item;
        }
      }

      return std::nullopt;
    }

    [[nodiscard]] std::optional<Monitor> find_by_slug(
        const std::string &slug) const override
    {
      for (const auto &item : monitors)
      {
        if (item.slug() == slug)
        {
          return item;
        }
      }

      return std::nullopt;
    }

    [[nodiscard]] std::vector<Monitor> list_all() const override
    {
      return monitors;
    }

    [[nodiscard]] bool exists_by_slug(const std::string &slug) const override
    {
      for (const auto &item : monitors)
      {
        if (item.slug() == slug)
        {
          return true;
        }
      }

      return false;
    }

    [[nodiscard]] bool remove_by_id(const EntityId &id) override
    {
      for (auto it = monitors.begin(); it != monitors.end(); ++it)
      {
        if (it->id() == id)
        {
          monitors.erase(it);
          return true;
        }
      }

      return false;
    }

    std::vector<Monitor> monitors;
  };

  class FakeCheckResultRepository final : public pulsegrid::application::ports::CheckResultRepository
  {
  public:
    using CheckResult = pulsegrid::domain::check::CheckResult;
    using EntityId = pulsegrid::domain::shared::EntityId;

    void create(const CheckResult &result) override
    {
      results.push_back(result);
    }

    [[nodiscard]] std::optional<CheckResult> find_by_id(
        const EntityId &id) const override
    {
      for (const auto &item : results)
      {
        if (item.id() == id)
        {
          return item;
        }
      }

      return std::nullopt;
    }

    [[nodiscard]] std::optional<CheckResult> find_latest_by_monitor_id(
        const EntityId &monitor_id) const override
    {
      std::optional<CheckResult> latest;

      for (const auto &item : results)
      {
        if (item.monitor_id() != monitor_id)
        {
          continue;
        }

        if (!latest.has_value() ||
            item.checked_at_ms() > latest->checked_at_ms())
        {
          latest = item;
        }
      }

      return latest;
    }

    [[nodiscard]] std::vector<CheckResult> list_by_monitor_id(
        const EntityId &monitor_id,
        std::size_t limit) const override
    {
      std::vector<CheckResult> out;

      for (const auto &item : results)
      {
        if (item.monitor_id() == monitor_id)
        {
          out.push_back(item);
          if (out.size() >= limit)
          {
            break;
          }
        }
      }

      return out;
    }

    [[nodiscard]] std::size_t remove_by_monitor_id(
        const EntityId &monitor_id) override
    {
      std::size_t removed = 0;

      for (auto it = results.begin(); it != results.end();)
      {
        if (it->monitor_id() == monitor_id)
        {
          it = results.erase(it);
          ++removed;
        }
        else
        {
          ++it;
        }
      }

      return removed;
    }

    std::vector<CheckResult> results;
  };

  class FakeIncidentRepository final : public pulsegrid::application::ports::IncidentRepository
  {
  public:
    using Incident = pulsegrid::domain::incident::Incident;
    using EntityId = pulsegrid::domain::shared::EntityId;

    void create(const Incident &incident) override
    {
      incidents.push_back(incident);
    }

    void update(const Incident &incident) override
    {
      for (auto &item : incidents)
      {
        if (item.id() == incident.id())
        {
          item = incident;
          return;
        }
      }

      incidents.push_back(incident);
    }

    [[nodiscard]] std::optional<Incident> find_by_id(
        const EntityId &id) const override
    {
      for (const auto &item : incidents)
      {
        if (item.id() == id)
        {
          return item;
        }
      }

      return std::nullopt;
    }

    [[nodiscard]] std::optional<Incident> find_open_by_monitor_id(
        const EntityId &monitor_id) const override
    {
      for (const auto &item : incidents)
      {
        if (item.monitor_id() == monitor_id && item.is_open())
        {
          return item;
        }
      }

      return std::nullopt;
    }

    [[nodiscard]] std::vector<Incident> list_by_monitor_id(
        const EntityId &monitor_id,
        std::size_t limit) const override
    {
      std::vector<Incident> out;

      for (const auto &item : incidents)
      {
        if (item.monitor_id() == monitor_id)
        {
          out.push_back(item);
          if (out.size() >= limit)
          {
            break;
          }
        }
      }

      return out;
    }

    [[nodiscard]] std::size_t remove_by_monitor_id(
        const EntityId &monitor_id) override
    {
      std::size_t removed = 0;

      for (auto it = incidents.begin(); it != incidents.end();)
      {
        if (it->monitor_id() == monitor_id)
        {
          it = incidents.erase(it);
          ++removed;
        }
        else
        {
          ++it;
        }
      }

      return removed;
    }

    std::vector<Incident> incidents;
  };

  pulsegrid::domain::monitor::Monitor make_monitor(
      std::string id,
      std::string name,
      std::string slug,
      std::string url,
      pulsegrid::domain::monitor::MonitorStatus status,
      std::int64_t ts)
  {
    return pulsegrid::domain::monitor::Monitor(
        pulsegrid::domain::shared::EntityId(std::move(id)),
        std::move(name),
        std::move(slug),
        pulsegrid::domain::shared::Url(std::move(url)),
        pulsegrid::domain::monitor::CheckInterval(60),
        status,
        ts,
        ts);
  }

  pulsegrid::domain::check::CheckResult make_success_check(
      std::string id,
      std::string monitor_id,
      std::int64_t response_time_ms,
      std::int64_t checked_at_ms)
  {
    return pulsegrid::domain::check::CheckResult::success(
        pulsegrid::domain::shared::EntityId(std::move(id)),
        pulsegrid::domain::shared::EntityId(std::move(monitor_id)),
        pulsegrid::domain::check::ResponseTime(response_time_ms),
        checked_at_ms);
  }

  pulsegrid::domain::incident::Incident make_open_incident(
      std::string id,
      std::string monitor_id,
      std::string message,
      std::int64_t started_at_ms)
  {
    return pulsegrid::domain::incident::Incident::open(
        pulsegrid::domain::shared::EntityId(std::move(id)),
        pulsegrid::domain::shared::EntityId(std::move(monitor_id)),
        std::move(message),
        started_at_ms);
  }

} // namespace

int main()
{
  using namespace vix::tests;
  using pulsegrid::application::services::StatusService;

  TestRegistry::instance().add(
      TestCase("StatusService get_system_summary aggregates monitor counts", []
               {
                 FakeMonitorRepository monitor_repository;
                 FakeCheckResultRepository check_result_repository;
                 FakeIncidentRepository incident_repository;

                 monitor_repository.monitors.push_back(
                     make_monitor(
                         "m1",
                         "Main API",
                         "main-api",
                         "https://example.com",
                         pulsegrid::domain::monitor::MonitorStatus::Up,
                         1000));

                 monitor_repository.monitors.push_back(
                     make_monitor(
                         "m2",
                         "Payments",
                         "payments",
                         "https://payments.example.com",
                         pulsegrid::domain::monitor::MonitorStatus::Down,
                         1000));

                 monitor_repository.monitors.push_back(
                     make_monitor(
                         "m3",
                         "Workers",
                         "workers",
                         "https://workers.example.com",
                         pulsegrid::domain::monitor::MonitorStatus::Degraded,
                         1000));

                 monitor_repository.monitors.push_back(
                     make_monitor(
                         "m4",
                         "Backoffice",
                         "backoffice",
                         "https://backoffice.example.com",
                         pulsegrid::domain::monitor::MonitorStatus::Paused,
                         1000));

                 incident_repository.incidents.push_back(
                     make_open_incident("inc-1", "m2", "Service unavailable", 2000));

                 incident_repository.incidents.push_back(
                     make_open_incident("inc-2", "m3", "Latency issue", 2500));

                 StatusService service(
                     monitor_repository,
                     check_result_repository,
                     incident_repository);

                 const auto summary = service.get_system_summary();

                 Assert::equal(summary.total_monitors, static_cast<std::size_t>(4));
                 Assert::equal(summary.up_monitors, static_cast<std::size_t>(1));
                 Assert::equal(summary.down_monitors, static_cast<std::size_t>(1));
                 Assert::equal(summary.degraded_monitors, static_cast<std::size_t>(1));
                 Assert::equal(summary.paused_monitors, static_cast<std::size_t>(1));
                 Assert::equal(summary.open_incidents, static_cast<std::size_t>(2));
                 Assert::equal(summary.monitors.size(), static_cast<std::size_t>(4)); }));

  TestRegistry::instance().add(
      TestCase("StatusService get_monitor_status_by_slug returns latest check and open incident", []
               {
                 FakeMonitorRepository monitor_repository;
                 FakeCheckResultRepository check_result_repository;
                 FakeIncidentRepository incident_repository;

                 monitor_repository.monitors.push_back(
                     make_monitor(
                         "m1",
                         "Main API",
                         "main-api",
                         "https://example.com",
                         pulsegrid::domain::monitor::MonitorStatus::Up,
                         1000));

                 check_result_repository.results.push_back(
                     make_success_check("c1", "m1", 120, 2000));

                 check_result_repository.results.push_back(
                     make_success_check("c2", "m1", 95, 3000));

                 incident_repository.incidents.push_back(
                     make_open_incident("inc-1", "m1", "Intermittent errors", 2500));

                 StatusService service(
                     monitor_repository,
                     check_result_repository,
                     incident_repository);

                 const auto view = service.get_monitor_status_by_slug("main-api");

                 Assert::equal(view.monitor.id().value(), std::string("m1"));
                 Assert::is_true(view.latest_check.has_value());
                 Assert::is_true(view.open_incident.has_value());
                 Assert::equal(view.latest_check->id().value(), std::string("c2"));
                 Assert::equal(view.latest_check->response_time()->milliseconds(), static_cast<std::int64_t>(95));
                 Assert::equal(view.open_incident->id().value(), std::string("inc-1")); }));

  TestRegistry::instance().add(
      TestCase("StatusService list_monitor_status_views returns one view per monitor", []
               {
                 FakeMonitorRepository monitor_repository;
                 FakeCheckResultRepository check_result_repository;
                 FakeIncidentRepository incident_repository;

                 monitor_repository.monitors.push_back(
                     make_monitor(
                         "m1",
                         "Main API",
                         "main-api",
                         "https://example.com",
                         pulsegrid::domain::monitor::MonitorStatus::Up,
                         1000));

                 monitor_repository.monitors.push_back(
                     make_monitor(
                         "m2",
                         "Search",
                         "search",
                         "https://search.example.com",
                         pulsegrid::domain::monitor::MonitorStatus::Unknown,
                         1000));

                 check_result_repository.results.push_back(
                     make_success_check("c1", "m1", 110, 2000));

                 StatusService service(
                     monitor_repository,
                     check_result_repository,
                     incident_repository);

                 const auto views = service.list_monitor_status_views();

                 Assert::equal(views.size(), static_cast<std::size_t>(2));
                 Assert::equal(views[0].monitor.id().value(), std::string("m1"));
                 Assert::equal(views[1].monitor.id().value(), std::string("m2"));
                 Assert::is_true(views[0].latest_check.has_value()); }));

  return TestRunner::run_all_and_exit();
}
