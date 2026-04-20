/**
 *
 *  @file MonitorRepository.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Monitor repository port.
 *
 */

#ifndef PULSEGRID_APPLICATION_PORTS_MONITOR_REPOSITORY_HPP
#define PULSEGRID_APPLICATION_PORTS_MONITOR_REPOSITORY_HPP

#include <optional>
#include <string>
#include <vector>

#include <pulsegrid/domain/monitor/Monitor.hpp>
#include <pulsegrid/domain/shared/EntityId.hpp>

namespace pulsegrid::application::ports
{
  class MonitorRepository
  {
  public:
    virtual ~MonitorRepository() = default;

    /**
     * @brief Persist a new monitor.
     */
    virtual void create(const pulsegrid::domain::monitor::Monitor &monitor) = 0;

    /**
     * @brief Persist updates to an existing monitor.
     */
    virtual void update(const pulsegrid::domain::monitor::Monitor &monitor) = 0;

    /**
     * @brief Find a monitor by id.
     */
    [[nodiscard]] virtual std::optional<pulsegrid::domain::monitor::Monitor> find_by_id(
        const pulsegrid::domain::shared::EntityId &id) const = 0;

    /**
     * @brief Find a monitor by slug.
     */
    [[nodiscard]] virtual std::optional<pulsegrid::domain::monitor::Monitor> find_by_slug(
        const std::string &slug) const = 0;

    /**
     * @brief Return all monitors.
     */
    [[nodiscard]] virtual std::vector<pulsegrid::domain::monitor::Monitor> list_all() const = 0;

    /**
     * @brief Return true if a monitor with this slug already exists.
     */
    [[nodiscard]] virtual bool exists_by_slug(const std::string &slug) const = 0;

    /**
     * @brief Delete a monitor by id.
     *
     * @return true if a monitor was deleted.
     */
    [[nodiscard]] virtual bool remove_by_id(
        const pulsegrid::domain::shared::EntityId &id) = 0;
  };

} // namespace pulsegrid::application::ports

#endif // PULSEGRID_APPLICATION_PORTS_MONITOR_REPOSITORY_HPP
