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
     * @brief Broadcast a payload to all connected websocket sessions.
     */
    void broadcast(const std::string &payload);

    /**
     * @brief Number of tracked live sessions.
     */
    [[nodiscard]] std::size_t connection_count() const;

  private:
    void handle_open(vix::websocket::Session &session);
    void handle_close(vix::websocket::Session &session);
    void handle_message(
        vix::websocket::Session &session,
        const std::string &type,
        const vix::json::kvs &payload);

    void prune_closed_sessions();

  private:
    vix::websocket::App &ws_app_;
    std::vector<vix::websocket::Session *> sessions_;
    mutable std::mutex sessions_mutex_;
  };

} // namespace pulsegrid::presentation::ws

#endif // PULSEGRID_PRESENTATION_WS_STATUS_WS_GATEWAY_HPP
