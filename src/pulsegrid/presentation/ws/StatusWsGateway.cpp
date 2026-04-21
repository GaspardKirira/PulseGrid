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
#include <variant>

#include <pulsegrid/support/Constants.hpp>

namespace pulsegrid::presentation::ws
{
  namespace
  {
    void safe_send(vix::websocket::Session &session, const std::string &payload)
    {
      if (!session.is_open())
      {
        return;
      }

      try
      {
        session.send_text(payload);
      }
      catch (...)
      {
      }
    }
  } // namespace

  StatusWsGateway::StatusWsGateway(vix::websocket::App &ws_app)
      : ws_app_(ws_app)
  {
  }

  void StatusWsGateway::register_routes()
  {
    ws_app_.server().on_open(
        [this](vix::websocket::Session &session)
        {
          handle_open(session);
        });

    ws_app_.server().on_close(
        [this](vix::websocket::Session &session)
        {
          handle_close(session);
        });

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

    for (auto &entry : sessions_)
    {
      auto session = entry.session.lock();
      if (!session || !session->is_open())
      {
        continue;
      }

      safe_send(*session, payload);
    }
  }

  void StatusWsGateway::broadcast_dashboard(const std::string &payload)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    prune_closed_sessions();

    for (auto &entry : sessions_)
    {
      auto session = entry.session.lock();
      if (!session || !session->is_open())
      {
        continue;
      }

      if (entry.subscriptions.dashboard)
      {
        safe_send(*session, payload);
      }
    }
  }

  void StatusWsGateway::broadcast_monitor_id(
      const std::string &monitor_id,
      const std::string &payload)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    prune_closed_sessions();

    for (auto &entry : sessions_)
    {
      auto session = entry.session.lock();
      if (!session || !session->is_open())
      {
        continue;
      }

      if (entry.subscriptions.monitor_ids.find(monitor_id) != entry.subscriptions.monitor_ids.end())
      {
        safe_send(*session, payload);
      }
    }
  }

  void StatusWsGateway::broadcast_monitor_slug(
      const std::string &slug,
      const std::string &payload)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    prune_closed_sessions();

    for (auto &entry : sessions_)
    {
      auto session = entry.session.lock();
      if (!session || !session->is_open())
      {
        continue;
      }

      if (entry.subscriptions.monitor_slugs.find(slug) != entry.subscriptions.monitor_slugs.end())
      {
        safe_send(*session, payload);
      }
    }
  }

  std::size_t StatusWsGateway::connection_count() const
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    std::size_t count = 0;
    for (const auto &entry : sessions_)
    {
      auto session = entry.session.lock();
      if (session && session->is_open())
      {
        ++count;
      }
    }

    return count;
  }

  void StatusWsGateway::handle_open(vix::websocket::Session &session)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto shared = session.shared_from_this();

    const auto it = std::find_if(
        sessions_.begin(),
        sessions_.end(),
        [&session](const SessionEntry &entry)
        {
          auto sp = entry.session.lock();
          return sp && sp.get() == &session;
        });

    if (it == sessions_.end())
    {
      sessions_.push_back(SessionEntry{
          .session = shared,
          .subscriptions = SubscriptionState{},
      });
    }

    safe_send(session, R"({"type":"ws.connected","data":{"ok":true}})");
  }

  void StatusWsGateway::handle_close(vix::websocket::Session &session)
  {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    sessions_.erase(
        std::remove_if(
            sessions_.begin(),
            sessions_.end(),
            [&session](const SessionEntry &entry)
            {
              auto sp = entry.session.lock();
              return !sp || sp.get() == &session;
            }),
        sessions_.end());
  }

  void StatusWsGateway::handle_message(
      vix::websocket::Session &session,
      const std::string &type,
      const vix::json::kvs &payload)
  {
    if (type == "ping")
    {
      safe_send(session, R"({"type":"pong","data":{"ok":true}})");
      return;
    }

    if (type == "status.subscribe")
    {
      subscribe_dashboard(session);
      safe_send(session, R"({"type":"status.subscribed","data":{"scope":"dashboard","ok":true}})");
      return;
    }

    if (type == "status.unsubscribe")
    {
      unsubscribe_dashboard(session);
      safe_send(session, R"({"type":"status.unsubscribed","data":{"scope":"dashboard","ok":true}})");
      return;
    }

    if (type == "monitor.subscribe")
    {
      const std::string monitor_id = payload_string(payload, "monitor_id");
      const std::string slug = payload_string(payload, "slug");

      if (!monitor_id.empty())
      {
        subscribe_monitor_id(session, monitor_id);
        safe_send(
            session,
            std::string(R"({"type":"monitor.subscribed","data":{"scope":"monitor_id","value":")") +
                monitor_id +
                R"(","ok":true}})");
        return;
      }

      if (!slug.empty())
      {
        subscribe_monitor_slug(session, slug);
        safe_send(
            session,
            std::string(R"({"type":"monitor.subscribed","data":{"scope":"slug","value":")") +
                slug +
                R"(","ok":true}})");
        return;
      }

      safe_send(session, R"({"type":"monitor.subscribed","data":{"ok":false,"message":"missing monitor_id or slug"}})");
      return;
    }

    if (type == "monitor.unsubscribe")
    {
      const std::string monitor_id = payload_string(payload, "monitor_id");
      const std::string slug = payload_string(payload, "slug");

      if (!monitor_id.empty())
      {
        unsubscribe_monitor_id(session, monitor_id);
        safe_send(
            session,
            std::string(R"({"type":"monitor.unsubscribed","data":{"scope":"monitor_id","value":")") +
                monitor_id +
                R"(","ok":true}})");
        return;
      }

      if (!slug.empty())
      {
        unsubscribe_monitor_slug(session, slug);
        safe_send(
            session,
            std::string(R"({"type":"monitor.unsubscribed","data":{"scope":"slug","value":")") +
                slug +
                R"(","ok":true}})");
        return;
      }

      safe_send(session, R"({"type":"monitor.unsubscribed","data":{"ok":false,"message":"missing monitor_id or slug"}})");
      return;
    }

    safe_send(session, R"({"type":"ws.ignored","data":{"ok":false,"message":"unsupported event"}})");
  }

  void StatusWsGateway::prune_closed_sessions()
  {
    sessions_.erase(
        std::remove_if(
            sessions_.begin(),
            sessions_.end(),
            [](const SessionEntry &entry)
            {
              auto session = entry.session.lock();
              return !session || !session->is_open();
            }),
        sessions_.end());
  }

  void StatusWsGateway::subscribe_dashboard(vix::websocket::Session &session)
  {
    if (auto *state = find_subscription_state(session))
    {
      state->dashboard = true;
    }
  }

  void StatusWsGateway::unsubscribe_dashboard(vix::websocket::Session &session)
  {
    if (auto *state = find_subscription_state(session))
    {
      state->dashboard = false;
    }
  }

  void StatusWsGateway::subscribe_monitor_id(
      vix::websocket::Session &session,
      const std::string &monitor_id)
  {
    if (monitor_id.empty())
    {
      return;
    }

    if (auto *state = find_subscription_state(session))
    {
      state->monitor_ids.insert(monitor_id);
    }
  }

  void StatusWsGateway::unsubscribe_monitor_id(
      vix::websocket::Session &session,
      const std::string &monitor_id)
  {
    if (monitor_id.empty())
    {
      return;
    }

    if (auto *state = find_subscription_state(session))
    {
      state->monitor_ids.erase(monitor_id);
    }
  }

  void StatusWsGateway::subscribe_monitor_slug(
      vix::websocket::Session &session,
      const std::string &slug)
  {
    if (slug.empty())
    {
      return;
    }

    if (auto *state = find_subscription_state(session))
    {
      state->monitor_slugs.insert(slug);
    }
  }

  void StatusWsGateway::unsubscribe_monitor_slug(
      vix::websocket::Session &session,
      const std::string &slug)
  {
    if (slug.empty())
    {
      return;
    }

    if (auto *state = find_subscription_state(session))
    {
      state->monitor_slugs.erase(slug);
    }
  }

  StatusWsGateway::SubscriptionState *StatusWsGateway::find_subscription_state(
      vix::websocket::Session &session)
  {
    auto it = std::find_if(
        sessions_.begin(),
        sessions_.end(),
        [&session](SessionEntry &entry)
        {
          auto sp = entry.session.lock();
          return sp && sp.get() == &session;
        });

    if (it == sessions_.end())
    {
      return nullptr;
    }

    return &it->subscriptions;
  }

  const StatusWsGateway::SubscriptionState *StatusWsGateway::find_subscription_state(
      vix::websocket::Session &session) const
  {
    auto it = std::find_if(
        sessions_.begin(),
        sessions_.end(),
        [&session](const SessionEntry &entry)
        {
          auto sp = entry.session.lock();
          return sp && sp.get() == &session;
        });

    if (it == sessions_.end())
    {
      return nullptr;
    }

    return &it->subscriptions;
  }

  std::string StatusWsGateway::payload_string(
      const vix::json::kvs &payload,
      const std::string &key)
  {
    const auto &flat = payload.flat;

    for (std::size_t i = 0; i + 1 < flat.size(); i += 2)
    {
      const auto &k = flat[i].v;
      const auto &v = flat[i + 1].v;

      if (!std::holds_alternative<std::string>(k))
      {
        continue;
      }

      if (std::get<std::string>(k) != key)
      {
        continue;
      }

      if (std::holds_alternative<std::string>(v))
      {
        return std::get<std::string>(v);
      }

      return {};
    }

    return {};
  }

} // namespace pulsegrid::presentation::ws
