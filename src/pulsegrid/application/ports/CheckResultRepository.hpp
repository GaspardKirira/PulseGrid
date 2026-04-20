/**
 *
 *  @file CheckResultRepository.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Check result repository port.
 *
 */

#ifndef PULSEGRID_APPLICATION_PORTS_CHECK_RESULT_REPOSITORY_HPP
#define PULSEGRID_APPLICATION_PORTS_CHECK_RESULT_REPOSITORY_HPP

#include <optional>
#include <vector>

#include <pulsegrid/domain/check/CheckResult.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::application::ports
{
  class CheckResultRepository
  {
  public:
    virtual ~CheckResultRepository() = default;

    /**
     * @brief Persist a new check result.
     */
    virtual void create(const pulsegrid::domain::check::CheckResult &result) = 0;

    /**
     * @brief Find a check result by id.
     */
    [[nodiscard]] virtual std::optional<pulsegrid::domain::check::CheckResult> find_by_id(
        const pulsegrid::domain::shared::EntityId &id) const = 0;

    /**
     * @brief Return the most recent check result for a monitor.
     */
    [[nodiscard]] virtual std::optional<pulsegrid::domain::check::CheckResult> find_latest_by_monitor_id(
        const pulsegrid::domain::shared::EntityId &monitor_id) const = 0;

    /**
     * @brief Return recent check results for a monitor.
     */
    [[nodiscard]] virtual std::vector<pulsegrid::domain::check::CheckResult> list_by_monitor_id(
        const pulsegrid::domain::shared::EntityId &monitor_id,
        std::size_t limit) const = 0;

    /**
     * @brief Delete all check results for a monitor.
     *
     * @return number of deleted rows/entities.
     */
    [[nodiscard]] virtual std::size_t remove_by_monitor_id(
        const pulsegrid::domain::shared::EntityId &monitor_id) = 0;
  };

} // namespace pulsegrid::application::ports

#endif // PULSEGRID_APPLICATION_PORTS_CHECK_RESULT_REPOSITORY_HPP
