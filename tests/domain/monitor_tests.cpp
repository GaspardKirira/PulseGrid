/**
 *
 *  @file monitor_tests.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include <vix/tests/tests.hpp>

#include <pulsegrid/domain/monitor/Monitor.hpp>
#include <pulsegrid/domain/monitor/MonitorStatus.hpp>
#include <pulsegrid/domain/monitor/CheckInterval.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>
#include <pulsegrid/domain/shared/Url.hpp>

int main()
{
  using namespace vix::tests;
  using namespace pulsegrid::domain;

  TestRegistry::instance().add(
      TestCase("Monitor creation default values", []
               {
                 monitor::Monitor m(
                     shared::EntityId("m1"),
                     "API",
                     "api",
                     shared::Url("https://example.com"),
                     monitor::CheckInterval(30),
                     monitor::MonitorStatus::Unknown,
                     100,
                     100);

                 Assert::equal(m.name(), std::string("API"));
                 Assert::equal(m.slug(), std::string("api"));
                 Assert::equal(
                     monitor::to_string(m.status()),
                     std::string("unknown")); }));

  TestRegistry::instance().add(
      TestCase("Monitor mark_up changes status", []
               {
                 monitor::Monitor m(
                     shared::EntityId("m1"),
                     "API",
                     "api",
                     shared::Url("https://example.com"),
                     monitor::CheckInterval(30),
                     monitor::MonitorStatus::Unknown,
                     100,
                     100);

                 m.mark_up(200);

                 Assert::equal(
                     monitor::to_string(m.status()),
                     std::string("up")); }));

  TestRegistry::instance().add(
      TestCase("Monitor mark_down changes status", []
               {
                 monitor::Monitor m(
                     shared::EntityId("m1"),
                     "API",
                     "api",
                     shared::Url("https://example.com"),
                     monitor::CheckInterval(30),
                     monitor::MonitorStatus::Up,
                     100,
                     100);

                 m.mark_down(300);

                 Assert::equal(
                     monitor::to_string(m.status()),
                     std::string("down")); }));

  TestRegistry::instance().add(
      TestCase("Monitor mark_degraded changes status", []
               {
                 monitor::Monitor m(
                     shared::EntityId("m1"),
                     "API",
                     "api",
                     shared::Url("https://example.com"),
                     monitor::CheckInterval(30),
                     monitor::MonitorStatus::Up,
                     100,
                     100);

                 m.mark_degraded(400);

                 Assert::equal(
                     monitor::to_string(m.status()),
                     std::string("degraded")); }));

  return TestRunner::run_all_and_exit();
}
