/**
 *
 *  @file StatusWsGateway.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  WebSocket gateway for realtime status updates.
 *
 */

#ifndef PULSEGRID_PRESENTATION_WS_STATUS_WS_GATEWAY_HPP
#define PULSEGRID_PRESENTATION_WS_STATUS_WS_GATEWAY_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <vix/websocket/App.hpp>
#include <vix/websocket/session.hpp>

namespace pulsegrid::presentation::ws
{
  class StatusWsGateway
  {
  public:
    explicit StatusWsGateway(vix::websocket::App &ws_app);

    /**
     * @brief Register websocket routes and internal handlers.
     */
    void register_routes();

    /**
     * @brief Broadcast a raw payload to all connected websocket sessions.
     */
    void broadcast(const std::string &payload);

    /**
     * @brief Broadcast a raw payload only to dashboard subscribers.
     */
    void broadcast_dashboard(const std::string &payload);

    /**
     * @brief Broadcast a raw payload only to subscribers of a monitor id.
     */
    void broadcast_monitor_id(
        const std::string &monitor_id,
        const std::string &payload);

    /**
     * @brief Broadcast a raw payload only to subscribers of a monitor slug.
     */
    void broadcast_monitor_slug(
        const std::string &slug,
        const std::string &payload);

    /**
     * @brief Number of tracked live sessions.
     */
    [[nodiscard]] std::size_t connection_count() const;

  private:
    struct SubscriptionState
    {
      bool dashboard{false};
      std::unordered_set<std::string> monitor_ids;
      std::unordered_set<std::string> monitor_slugs;
    };

  private:
    void handle_open(vix::websocket::Session &session);
    void handle_close(vix::websocket::Session &session);
    void handle_message(
        vix::websocket::Session &session,
        const std::string &type,
        const vix::json::kvs &payload);

    void prune_closed_sessions();

    void subscribe_dashboard(vix::websocket::Session &session);
    void unsubscribe_dashboard(vix::websocket::Session &session);

    void subscribe_monitor_id(
        vix::websocket::Session &session,
        const std::string &monitor_id);

    void unsubscribe_monitor_id(
        vix::websocket::Session &session,
        const std::string &monitor_id);

    void subscribe_monitor_slug(
        vix::websocket::Session &session,
        const std::string &slug);

    void unsubscribe_monitor_slug(
        vix::websocket::Session &session,
        const std::string &slug);

    [[nodiscard]] SubscriptionState &subscription_state(vix::websocket::Session &session);

    [[nodiscard]] static std::string payload_string(
        const vix::json::kvs &payload,
        const std::string &key);

    template <typename Predicate>
    void broadcast_if(
        const std::string &payload,
        Predicate predicate);

  private:
    vix::websocket::App &ws_app_;
    std::vector<vix::websocket::Session *> sessions_;
    std::unordered_map<vix::websocket::Session *, SubscriptionState> subscriptions_;
    mutable std::mutex sessions_mutex_;
  };

} // namespace pulsegrid::presentation::ws

#endif // PULSEGRID_PRESENTATION_WS_STATUS_WS_GATEWAY_HPP
