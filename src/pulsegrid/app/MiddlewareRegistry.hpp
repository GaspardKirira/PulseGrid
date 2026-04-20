/**
 *
 *  @file MiddlewareRegistry.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Central middleware registration for the HTTP app.
 *
 */

#ifndef PULSEGRID_APP_MIDDLEWARE_REGISTRY_HPP
#define PULSEGRID_APP_MIDDLEWARE_REGISTRY_HPP

#include <vix.hpp>

namespace pulsegrid::app
{
  class MiddlewareRegistry
  {
  public:
    MiddlewareRegistry() = default;

    /**
     * @brief Install all application middlewares.
     */
    void register_all(vix::App &app) const;

  private:
    void register_global(vix::App &app) const;
    void register_api(vix::App &app) const;
  };

} // namespace pulsegrid::app

#endif // PULSEGRID_APP_MIDDLEWARE_REGISTRY_HPP
