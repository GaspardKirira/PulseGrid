/**
 *
 *  @file test_basic.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include <vix/tests/tests.hpp>

#include <pulsegrid/domain/check/ResponseTime.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/incident/IncidentState.hpp>
#include <pulsegrid/domain/monitor/CheckInterval.hpp>
#include <pulsegrid/domain/monitor/Monitor.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/domain/shared/Url.hpp>

int main()
{
  using namespace vix::tests;

  TestRegistry::instance().add(
      TestCase("EntityId stores value", []
               {
                 pulsegrid::domain::shared::EntityId id("abc-123");
                 Assert::equal(id.value(), std::string("abc-123")); }));

  TestRegistry::instance().add(
      TestCase("Url accepts https url", []
               {
                 pulsegrid::domain::shared::Url url("https://example.com");
                 Assert::equal(url.value(), std::string("https://example.com")); }));

  TestRegistry::instance().add(
      TestCase("CheckInterval stores seconds", []
               {
                 pulsegrid::domain::monitor::CheckInterval interval(60);
                 Assert::equal(interval.seconds(), 60); }));

  TestRegistry::instance().add(
      TestCase("ResponseTime stores milliseconds", []
               {
                 pulsegrid::domain::check::ResponseTime rt(250);
                 Assert::equal(rt.milliseconds(), static_cast<std::int64_t>(250)); }));

  TestRegistry::instance().add(
      TestCase("Monitor can be created", []
               {
                 pulsegrid::domain::monitor::Monitor monitor(
                     pulsegrid::domain::shared::EntityId("mon-1"),
                     "Main API",
                     "main-api",
                     pulsegrid::domain::shared::Url("https://example.com"),
                     pulsegrid::domain::monitor::CheckInterval(60),
                     pulsegrid::domain::monitor::MonitorStatus::Unknown,
                     1000,
                     1000);

                 Assert::equal(monitor.name(), std::string("Main API"));
                 Assert::equal(monitor.slug(), std::string("main-api"));
                 Assert::equal(
                     pulsegrid::domain::monitor::to_string(monitor.status()),
                     std::string("unknown")); }));

  TestRegistry::instance().add(
      TestCase("Incident open factory creates open incident", []
               {
                 auto incident = pulsegrid::domain::incident::Incident::open(
                     pulsegrid::domain::shared::EntityId("inc-1"),
                     pulsegrid::domain::shared::EntityId("mon-1"),
                     "Service unavailable",
                     2000);

                 Assert::is_true(incident.is_open());
                 Assert::is_true(!incident.resolved_at_ms().has_value()); }));

  return TestRunner::run_all_and_exit();
}
