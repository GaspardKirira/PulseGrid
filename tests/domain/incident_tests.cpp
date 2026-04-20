/**
 *
 *  @file incident_tests.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include <vix/tests/tests.hpp>

#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/incident/IncidentState.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

int main()
{
  using namespace vix::tests;
  using namespace pulsegrid::domain;

  TestRegistry::instance().add(
      TestCase("Incident open factory creates open incident", []
               {
                 auto incident = incident::Incident::open(
                     shared::EntityId("inc-1"),
                     shared::EntityId("mon-1"),
                     "Service unavailable",
                     1000);

                 Assert::is_true(incident.is_open());
                 Assert::is_true(!incident.resolved_at_ms().has_value());
                 Assert::equal(
                     incident::to_string(incident.state()),
                     std::string("open")); }));

  TestRegistry::instance().add(
      TestCase("Incident resolve changes state", []
               {
                 auto incident = incident::Incident::open(
                     shared::EntityId("inc-2"),
                     shared::EntityId("mon-2"),
                     "Timeout error",
                     1000);

                 incident.resolve(2000);

                 Assert::is_true(incident.is_resolved());
                 Assert::is_true(incident.resolved_at_ms().has_value());
                 Assert::equal(
                     *incident.resolved_at_ms(),
                     static_cast<std::int64_t>(2000));
                 Assert::equal(
                     incident::to_string(incident.state()),
                     std::string("resolved")); }));

  TestRegistry::instance().add(
      TestCase("Incident reopen clears resolution time", []
               {
                 auto incident = incident::Incident::open(
                     shared::EntityId("inc-3"),
                     shared::EntityId("mon-3"),
                     "Partial outage",
                     1000);

                 incident.resolve(2500);
                 incident.reopen();

                 Assert::is_true(incident.is_open());
                 Assert::is_true(!incident.resolved_at_ms().has_value());
                 Assert::equal(
                     incident::to_string(incident.state()),
                     std::string("open")); }));

  TestRegistry::instance().add(
      TestCase("Incident update_message changes message", []
               {
                 auto incident = incident::Incident::open(
                     shared::EntityId("inc-4"),
                     shared::EntityId("mon-4"),
                     "Initial message",
                     1000);

                 incident.update_message("Updated incident message");

                 Assert::equal(
                     incident.message(),
                     std::string("Updated incident message")); }));

  return TestRunner::run_all_and_exit();
}
