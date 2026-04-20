/**
 *
 *  @file CheckService.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Application service for monitor checks.
 *
 */

#ifndef PULSEGRID_APPLICATION_SERVICES_CHECK_SERVICE_HPP
#define PULSEGRID_APPLICATION_SERVICES_CHECK_SERVICE_HPP

#include <optional>
#include <string>
#include <vector>

#include <pulsegrid/application/ports/CheckResultRepository.hpp>
#include <pulsegrid/application/ports/Clock.hpp>
#include <pulsegrid/application/ports/EventBus.hpp>
#include <pulsegrid/application/ports/IdGenerator.hpp>
#include <pulsegrid/application/ports/IncidentRepository.hpp>
#include <pulsegrid/application/ports/MonitorRepository.hpp>
#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/check/ResponseTime.hpp>
#include <pulsegrid/domain/incident/Incident.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::application::services
{
  class CheckService
  {
  public:
    using EntityId = pulsegrid::domain::shared::EntityId;
    using CheckResult = pulsegrid::domain::check::CheckResult;
    using ResponseTime = pulsegrid::domain::check::ResponseTime;
    using Incident = pulsegrid::domain::incident::Incident;

    struct RecordSuccessInput
    {
      EntityId monitor_id;
      std::int64_t response_time_ms{0};
    };

    struct RecordDegradedInput
    {
      EntityId monitor_id;
      std::int64_t response_time_ms{0};
      std::string error_message;
    };

    struct RecordFailureInput
    {
      EntityId monitor_id;
      std::string error_message;
    };

    struct RecordedCheck
    {
      CheckResult result;
      std::optional<Incident> opened_incident;
      std::optional<Incident> resolved_incident;
    };

  public:
    CheckService(
        pulsegrid::application::ports::MonitorRepository &monitor_repository,
        pulsegrid::application::ports::CheckResultRepository &check_result_repository,
        pulsegrid::application::ports::IncidentRepository &incident_repository,
        pulsegrid::application::ports::Clock &clock,
        pulsegrid::application::ports::IdGenerator &id_generator,
        pulsegrid::application::ports::EventBus &event_bus);

    [[nodiscard]] RecordedCheck record_success(const RecordSuccessInput &input);

    [[nodiscard]] RecordedCheck record_degraded(const RecordDegradedInput &input);

    [[nodiscard]] RecordedCheck record_failure(const RecordFailureInput &input);

    [[nodiscard]] std::optional<CheckResult> get_latest_for_monitor(
        const EntityId &monitor_id) const;

    [[nodiscard]] std::vector<CheckResult> list_recent_for_monitor(
        const EntityId &monitor_id,
        std::size_t limit) const;

  private:
    [[nodiscard]] RecordedCheck finalize_success_result(
        const EntityId &monitor_id,
        CheckResult result);

    [[nodiscard]] RecordedCheck finalize_problem_result(
        const EntityId &monitor_id,
        CheckResult result,
        const std::string &incident_message);

  private:
    pulsegrid::application::ports::MonitorRepository &monitor_repository_;
    pulsegrid::application::ports::CheckResultRepository &check_result_repository_;
    pulsegrid::application::ports::IncidentRepository &incident_repository_;
    pulsegrid::application::ports::Clock &clock_;
    pulsegrid::application::ports::IdGenerator &id_generator_;
    pulsegrid::application::ports::EventBus &event_bus_;
  };

} // namespace pulsegrid::application::services

#endif // PULSEGRID_APPLICATION_SERVICES_CHECK_SERVICE_HPP
