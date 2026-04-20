/**
 *
 *  @file StatusPageController.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  HTTP controller for public status endpoints.
 *
 */

#ifndef PULSEGRID_PRESENTATION_HTTP_STATUS_PAGE_CONTROLLER_HPP
#define PULSEGRID_PRESENTATION_HTTP_STATUS_PAGE_CONTROLLER_HPP

#include <pulsegrid/application/services/StatusService.hpp>
#include <vix.hpp>

namespace pulsegrid::presentation::http
{
  class StatusPageController
  {
  public:
    explicit StatusPageController(
        pulsegrid::application::services::StatusService &status_service);

    /**
     * @brief Register status-related HTTP routes.
     */
    void register_routes(vix::App &app) const;

  private:
    pulsegrid::application::services::StatusService &status_service_;
  };

} // namespace pulsegrid::presentation::http

#endif // PULSEGRID_PRESENTATION_HTTP_STATUS_PAGE_CONTROLLER_HPP
