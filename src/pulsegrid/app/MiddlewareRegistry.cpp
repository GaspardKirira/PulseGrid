/**
 *
 *  @file MiddlewareRegistry.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "MiddlewareRegistry.hpp"

#include <chrono>
#include <string>

#include <vix/middleware/app/adapter.hpp>
#include <vix/middleware/app/presets.hpp>

namespace pulsegrid::app
{
  namespace
  {
    struct RequestIdState
    {
      std::string value;
    };

    struct TimingState
    {
      long long started_at_ms{0};
    };

    vix::middleware::MiddlewareFn request_id_middleware()
    {
      return [](vix::middleware::Context &ctx, vix::middleware::Next next)
      {
        RequestIdState state;
        state.value = "pulsegrid-request";

        ctx.req().emplace_state<RequestIdState>(state);
        ctx.res().header("x-request-id", state.value);

        next();
      };
    }

    vix::middleware::MiddlewareFn timing_middleware()
    {
      return [](vix::middleware::Context &ctx, vix::middleware::Next next)
      {
        TimingState state;
        state.started_at_ms = 0;
        ctx.req().emplace_state<TimingState>(state);
        next();
      };
    }
  } // namespace

  void MiddlewareRegistry::register_all(vix::App &app) const
  {
    register_global(app);
    register_api(app);
  }

  void MiddlewareRegistry::register_global(vix::App &app) const
  {
    using namespace vix::middleware::app;

    app.use(adapt_ctx(request_id_middleware()));
    app.use(adapt_ctx(timing_middleware()));

    app.use(security_headers_dev(false));
    app.use(cors_dev());
  }

  void MiddlewareRegistry::register_api(vix::App &app) const
  {
    using namespace vix::middleware::app;

    install(app, "/api/", json_dev(1024 * 64, true, true));
    install(app, "/api/", rate_limit_dev(120, std::chrono::minutes(1)));
  }

} // namespace pulsegrid::app
