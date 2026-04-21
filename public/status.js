const STATUS_WS = {
  endpoint: "/",
  reconnectDelayMs: 3000,
};

const statusState = {
  ws: null,
  reconnectTimer: null,
  currentView: null,
};

function statusQs(id) {
  return document.getElementById(id);
}

function getSlugFromPath() {
  const parts = window.location.pathname.split("/").filter(Boolean);
  return parts.length >= 2 ? decodeURIComponent(parts[1]) : "";
}

function formatTimestamp(ms) {
  if (ms === null || ms === undefined) {
    return "—";
  }

  const numeric = Number(ms);
  if (!Number.isFinite(numeric)) {
    return "—";
  }

  const date = new Date(numeric);
  if (Number.isNaN(date.getTime())) {
    return "—";
  }

  return date.toLocaleString();
}

function formatResponseTime(ms) {
  if (ms === null || ms === undefined) {
    return "—";
  }

  return `${ms} ms`;
}

function statusClass(status) {
  switch ((status || "").toLowerCase()) {
    case "up":
      return "status-up";
    case "down":
      return "status-down";
    case "degraded":
      return "status-degraded";
    case "paused":
      return "status-paused";
    default:
      return "status-unknown";
  }
}

function showError(message) {
  const box = statusQs("error-box");
  if (!box) {
    return;
  }

  box.textContent = message || "Unknown error";
  box.classList.remove("hidden");
}

function hideError() {
  const box = statusQs("error-box");
  if (!box) {
    return;
  }

  box.classList.add("hidden");
}

function renderStatus(view) {
  statusState.currentView = view;

  const monitor = view?.monitor || {};
  const latestCheck = view?.latest_check || null;
  const openIncident = view?.open_incident || null;

  const hero = statusQs("hero");
  const metrics = statusQs("metrics");
  const incidentBox = statusQs("incident-box");

  document.title = `${monitor.name || "Monitor"} · PulseGrid Status`;

  const resolvedStatus = monitor.status || latestCheck?.status || "unknown";

  statusQs("monitor-name").textContent = monitor.name || "Unnamed monitor";
  statusQs("monitor-url").textContent = monitor.url || "";
  statusQs("monitor-meta").textContent =
    `Slug: ${monitor.slug || "—"} • Interval: ${monitor.interval_seconds ?? "—"}s`;

  const pill = statusQs("monitor-status-pill");
  pill.textContent = resolvedStatus.toUpperCase();
  pill.className = `status-pill ${statusClass(resolvedStatus)}`;

  statusQs("latest-check-status").textContent = latestCheck?.status
    ? latestCheck.status.toUpperCase()
    : "—";

  statusQs("response-time").textContent = formatResponseTime(
    latestCheck?.response_time_ms,
  );

  statusQs("checked-at").textContent = formatTimestamp(
    latestCheck?.checked_at_ms,
  );

  if (openIncident) {
    statusQs("incident-message").textContent =
      openIncident.message || "Incident open";
    statusQs("incident-started").textContent =
      `Started at: ${formatTimestamp(openIncident.started_at_ms)}`;
    incidentBox.classList.remove("hidden");
  } else {
    incidentBox.classList.add("hidden");
  }

  hero.classList.remove("hidden");
  metrics.classList.remove("hidden");
}

async function loadStatus() {
  hideError();

  const slug = getSlugFromPath();
  if (!slug) {
    showError("Missing monitor slug in URL.");
    return;
  }

  try {
    const response = await fetch(`/api/status/${encodeURIComponent(slug)}`);

    if (!response.ok) {
      const data = await response.json().catch(() => ({}));
      showError(data?.error || "Failed to load public status.");
      return;
    }

    const data = await response.json();

    if (!data || !data.monitor) {
      showError("Invalid response format.");
      return;
    }

    renderStatus(data);
  } catch (error) {
    console.error("[PulseGrid Status] loadStatus failed:", error);
    showError("Network error while loading status.");
  }
}

function wsUrl() {
  const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
  const host = window.location.hostname;
  return `${protocol}//${host}:9090${STATUS_WS.endpoint}`;
}

function mergeMonitorUpdate(data) {
  const previousView = statusState.currentView || {};
  const previousMonitor = previousView.monitor || {};
  const previousLatestCheck = previousView.latest_check || null;
  const previousIncident = previousView.open_incident || null;

  const hasLiveCheckData =
    data.response_time_ms !== undefined || data.checked_at_ms !== undefined;

  const nextLatestCheck = hasLiveCheckData
    ? {
        status:
          data.status ?? previousLatestCheck?.status ?? previousMonitor.status,
        response_time_ms:
          data.response_time_ms !== undefined
            ? data.response_time_ms
            : previousLatestCheck?.response_time_ms,
        checked_at_ms:
          data.checked_at_ms !== undefined
            ? data.checked_at_ms
            : previousLatestCheck?.checked_at_ms,
      }
    : previousLatestCheck;

  return {
    monitor: {
      ...previousMonitor,
      id: data.id ?? previousMonitor.id,
      name: data.name ?? previousMonitor.name,
      slug: data.slug ?? previousMonitor.slug,
      url: data.url ?? previousMonitor.url,
      interval_seconds:
        data.interval_seconds ?? previousMonitor.interval_seconds,
      status: data.status ?? previousMonitor.status,
      created_at_ms: data.created_at_ms ?? previousMonitor.created_at_ms,
      updated_at_ms: data.updated_at_ms ?? previousMonitor.updated_at_ms,
    },
    latest_check: nextLatestCheck,
    open_incident: previousIncident,
  };
}

function mergeCheckRecorded(data) {
  const previousView = statusState.currentView || {};
  const previousMonitor = previousView.monitor || {};

  return {
    monitor: {
      ...previousMonitor,
      status: data.status ?? previousMonitor.status,
    },
    latest_check: {
      status: data.status,
      response_time_ms: data.response_time_ms,
      checked_at_ms: data.checked_at_ms,
    },
    open_incident: previousView.open_incident || null,
  };
}

function mergeIncidentOpened(data) {
  const previousView = statusState.currentView || {};

  return {
    monitor: previousView.monitor || {},
    latest_check: previousView.latest_check || null,
    open_incident: {
      id: data.id,
      monitor_id: data.monitor_id,
      message: data.message,
      started_at_ms: data.started_at_ms,
      resolved_at_ms: data.resolved_at_ms,
      state: data.state,
    },
  };
}

function mergeIncidentResolved() {
  const previousView = statusState.currentView || {};

  return {
    monitor: previousView.monitor || {},
    latest_check: previousView.latest_check || null,
    open_incident: null,
  };
}

function handleWsMessage(raw) {
  let msg;

  try {
    msg = JSON.parse(raw);
  } catch {
    return;
  }

  if (!msg || typeof msg !== "object") {
    return;
  }

  if (
    msg.type === "ws.connected" ||
    msg.type === "monitor.subscribed" ||
    msg.type === "pong"
  ) {
    return;
  }

  const currentSlug = getSlugFromPath();
  const currentMonitorId = statusState.currentView?.monitor?.id || null;

  if (msg.type === "monitor.updated" && msg.data) {
    if (msg.data.slug !== currentSlug) {
      return;
    }

    renderStatus(mergeMonitorUpdate(msg.data));
    hideError();
    return;
  }

  if (msg.type === "check.recorded" && msg.data) {
    if (!currentMonitorId || msg.data.monitor_id !== currentMonitorId) {
      return;
    }

    renderStatus(mergeCheckRecorded(msg.data));
    hideError();
    return;
  }

  if (msg.type === "incident.opened" && msg.data) {
    if (!currentMonitorId || msg.data.monitor_id !== currentMonitorId) {
      return;
    }

    renderStatus(mergeIncidentOpened(msg.data));
    hideError();
    return;
  }

  if (msg.type === "incident.resolved" && msg.data) {
    if (!currentMonitorId || msg.data.monitor_id !== currentMonitorId) {
      return;
    }

    renderStatus(mergeIncidentResolved());
    hideError();
  }
}

function scheduleReconnect() {
  if (statusState.reconnectTimer) {
    return;
  }

  statusState.reconnectTimer = window.setTimeout(() => {
    statusState.reconnectTimer = null;
    connectWebSocket();
  }, STATUS_WS.reconnectDelayMs);
}

function connectWebSocket() {
  if (
    statusState.ws &&
    (statusState.ws.readyState === WebSocket.OPEN ||
      statusState.ws.readyState === WebSocket.CONNECTING)
  ) {
    return;
  }

  try {
    const socket = new WebSocket(wsUrl());
    statusState.ws = socket;

    socket.addEventListener("open", () => {
      const slug = getSlugFromPath();

      try {
        socket.send(
          JSON.stringify({
            type: "monitor.subscribe",
            payload: { slug },
          }),
        );
      } catch {
        // ignore
      }
    });

    socket.addEventListener("message", (event) => {
      handleWsMessage(event.data);
    });

    socket.addEventListener("close", () => {
      statusState.ws = null;
      scheduleReconnect();
    });

    socket.addEventListener("error", () => {
      if (statusState.ws) {
        statusState.ws.close();
      }
    });
  } catch {
    scheduleReconnect();
  }
}

document.addEventListener("DOMContentLoaded", () => {
  loadStatus();
  connectWebSocket();
});
