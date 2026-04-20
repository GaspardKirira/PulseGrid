/**
 *
 *  @file StatusWsGateway.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "StatusWsGateway.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include <pulsegrid/support/Constants.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <vix/json/Simple.hpp>

namespace pulsegrid::presentation::ws
{
  StatusWsGateway::StatusWsGateway(vix::websocket::App &ws_app)
      : ws_app_(ws_app)
  {
  }

  void StatusWsGateway::register_routes()
  {
    ws_app_.server().on_open([this](vix::websocket::Session &session)
                             { handle_open(session); });

    ws_app_.server().on_close([this](vix::websocket::Session &session)
                              { handle_close(session); });

    ws_app_.ws(
        std::string(pulsegrid::support::constants::ws_status_endpoint),
        [this](
            vix::websocket::Session &session,
            const std::string &type,
            const vix::json::kvs &payload)
        {
          handle_message(session, type, payload);
        });
  }

  void StatusWsGateway::broadcast(const std::string &payload)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    prune_closed_sessions();

    for (auto *session : sessions_)
    {
      if (session == nullptr)
      {
        continue;
      }

      try
      {
        session->send_text(payload);
      }
      catch (...)
      {
        // Ignore individual send failures in MVP broadcast path.
      }
    }
  }

  std::size_t StatusWsGateway::connection_count() const
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    return sessions_.size();
  }

  void StatusWsGateway::handle_open(vix::websocket::Session &session)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    sessions_.push_back(&session);

    try
    {
      session.send_text(R"({"type":"ws.connected","data":{"ok":true}})");
    }
    catch (...)
    {
    }
  }

  void StatusWsGateway::handle_close(vix::websocket::Session &session)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    sessions_.erase(
        std::remove(sessions_.begin(), sessions_.end(), &session),
        sessions_.end());
  }

  void StatusWsGateway::handle_message(
      vix::websocket::Session &session,
      const std::string &type,
      const vix::json::kvs &)
  {
    if (type == "ping")
    {
      try
      {
        session.send_text(R"({"type":"pong","data":{"ok":true}})");
      }
      catch (...)
      {
      }
      return;
    }

    if (type == "status.subscribe")
    {
      try
      {
        session.send_text(R"({"type":"status.subscribed","data":{"ok":true}})");
      }
      catch (...)
      {
      }
      return;
    }

    try
    {
      session.send_text(R"({"type":"ws.ignored","data":{"ok":false,"message":"unsupported event"}})");
    }
    catch (...)
    {
    }
  }

  void StatusWsGateway::prune_closed_sessions()
  {
    sessions_.erase(
        std::remove(sessions_.begin(), sessions_.end(), nullptr),
        sessions_.end());
  }

} // namespace pulsegrid::presentation::ws
