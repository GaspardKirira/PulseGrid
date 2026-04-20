/**
 *
 *  @file monitor_service_tests.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include <vix/tests/tests.hpp>

#include <optional>
#include <string>
#include <vector>

#include <pulsegrid/application/ports/Clock.hpp>
#include <pulsegrid/application/ports/EventBus.hpp>
#include <pulsegrid/application/ports/IdGenerator.hpp>
#include <pulsegrid/application/ports/MonitorRepository.hpp>
#include <pulsegrid/application/services/MonitorService.hpp>
#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/monitor/Monitor.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace
{
  class FakeClock final : public pulsegrid::application::ports::Clock
  {
  public:
    explicit FakeClock(std::int64_t now) : now_(now) {}

    [[nodiscard]] std::int64_t now_ms() const override
    {
      return now_;
    }

    void set_now(std::int64_t now)
    {
      now_ = now;
    }

  private:
    std::int64_t now_;
  };

  class FakeIdGenerator final : public pulsegrid::application::ports::IdGenerator
  {
  public:
    explicit FakeIdGenerator(std::string value)
        : next_value_(std::move(value))
    {
    }

    [[nodiscard]] pulsegrid::domain::shared::EntityId next() override
    {
      return pulsegrid::domain::shared::EntityId(next_value_);
    }

    void set_next(std::string value)
    {
      next_value_ = std::move(value);
    }

  private:
    std::string next_value_;
  };

  class FakeEventBus final : public pulsegrid::application::ports::EventBus
  {
  public:
    void publish_monitor_created(
        const pulsegrid::domain::monitor::Monitor &) override
    {
      ++monitor_created_count;
    }

    void publish_monitor_updated(
        const pulsegrid::domain::monitor::Monitor &) override
    {
      ++monitor_updated_count;
    }

    void publish_check_recorded(
        const pulsegrid::domain::check::CheckResult &) override
    {
      ++check_recorded_count;
    }

    void publish_incident_opened(
        const pulsegrid::domain::incident::Incident &) override
    {
      ++incident_opened_count;
    }

    void publish_incident_resolved(
        const pulsegrid::domain::incident::Incident &) override
    {
      ++incident_resolved_count;
    }

    int monitor_created_count{0};
    int monitor_updated_count{0};
    int check_recorded_count{0};
    int incident_opened_count{0};
    int incident_resolved_count{0};
  };

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

} // namespace

int main()
{
  using namespace vix::tests;
  using pulsegrid::application::services::MonitorService;
  using pulsegrid::domain::shared::EntityId;

  TestRegistry::instance().add(
      TestCase("MonitorService create_monitor persists and publishes event", []
               {
                 FakeMonitorRepository repository;
                 FakeClock clock(1000);
                 FakeIdGenerator id_generator("mon-1");
                 FakeEventBus event_bus;

                 MonitorService service(
                     repository,
                     clock,
                     id_generator,
                     event_bus);

                 const auto created = service.create_monitor({
                     .name = "Main API",
                     .slug = "main-api",
                     .url = "https://example.com",
                     .interval_seconds = 60,
                 });

                 Assert::equal(created.id().value(), std::string("mon-1"));
                 Assert::equal(created.name(), std::string("Main API"));
                 Assert::equal(created.slug(), std::string("main-api"));
                 Assert::equal(created.url().value(), std::string("https://example.com"));
                 Assert::equal(created.interval().seconds(), 60);
                 Assert::equal(repository.monitors.size(), static_cast<std::size_t>(1));
                 Assert::equal(event_bus.monitor_created_count, 1); }));

  TestRegistry::instance().add(
      TestCase("MonitorService update_monitor changes persisted monitor", []
               {
                 FakeMonitorRepository repository;
                 FakeClock clock(1000);
                 FakeIdGenerator id_generator("mon-1");
                 FakeEventBus event_bus;

                 MonitorService service(
                     repository,
                     clock,
                     id_generator,
                     event_bus);

                 const auto created = service.create_monitor({
                     .name = "Main API",
                     .slug = "main-api",
                     .url = "https://example.com",
                     .interval_seconds = 60,
                 });

                 clock.set_now(2000);

                 const auto updated = service.update_monitor(
                     created.id(),
                     {
                         .name = "Public API",
                         .slug = "public-api",
                         .url = "https://status.example.com",
                         .interval_seconds = 120,
                     });

                 Assert::equal(updated.name(), std::string("Public API"));
                 Assert::equal(updated.slug(), std::string("public-api"));
                 Assert::equal(updated.url().value(), std::string("https://status.example.com"));
                 Assert::equal(updated.interval().seconds(), 120);
                 Assert::equal(updated.updated_at_ms(), static_cast<std::int64_t>(2000));
                 Assert::equal(event_bus.monitor_updated_count, 1); }));

  TestRegistry::instance().add(
      TestCase("MonitorService pause_monitor marks monitor paused", []
               {
                 FakeMonitorRepository repository;
                 FakeClock clock(1000);
                 FakeIdGenerator id_generator("mon-1");
                 FakeEventBus event_bus;

                 MonitorService service(
                     repository,
                     clock,
                     id_generator,
                     event_bus);

                 const auto created = service.create_monitor({
                     .name = "Main API",
                     .slug = "main-api",
                     .url = "https://example.com",
                     .interval_seconds = 60,
                 });

                 clock.set_now(3000);

                 const auto paused = service.pause_monitor(created.id());

                 Assert::equal(
                     pulsegrid::domain::monitor::to_string(paused.status()),
                     std::string("paused"));
                 Assert::is_true(paused.is_paused()); }));

  TestRegistry::instance().add(
      TestCase("MonitorService resume_monitor marks monitor unknown", []
               {
                 FakeMonitorRepository repository;
                 FakeClock clock(1000);
                 FakeIdGenerator id_generator("mon-1");
                 FakeEventBus event_bus;

                 MonitorService service(
                     repository,
                     clock,
                     id_generator,
                     event_bus);

                 const auto created = service.create_monitor({
                     .name = "Main API",
                     .slug = "main-api",
                     .url = "https://example.com",
                     .interval_seconds = 60,
                 });

                 clock.set_now(2000);
                 (void)service.pause_monitor(created.id());

                 clock.set_now(4000);
                 const auto resumed = service.resume_monitor(created.id());

                 Assert::equal(
                     pulsegrid::domain::monitor::to_string(resumed.status()),
                     std::string("unknown"));
                 Assert::is_true(!resumed.is_paused()); }));

  TestRegistry::instance().add(
      TestCase("MonitorService delete_monitor removes entity", []
               {
                 FakeMonitorRepository repository;
                 FakeClock clock(1000);
                 FakeIdGenerator id_generator("mon-1");
                 FakeEventBus event_bus;

                 MonitorService service(
                     repository,
                     clock,
                     id_generator,
                     event_bus);

                 const auto created = service.create_monitor({
                     .name = "Main API",
                     .slug = "main-api",
                     .url = "https://example.com",
                     .interval_seconds = 60,
                 });

                 const bool removed = service.delete_monitor(created.id());

                 Assert::is_true(removed);
                 Assert::equal(repository.monitors.size(), static_cast<std::size_t>(0)); }));

  return TestRunner::run_all_and_exit();
}
