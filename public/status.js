const STATUS_WS = { reconnectDelayMs: 3000 };

const statusState = { ws: null, reconnectTimer: null, currentView: null };

function id(s) {
  return document.getElementById(s);
}
function qs(s) {
  return document.querySelector(s);
}

function getSlugFromPath() {
  const parts = window.location.pathname.split("/").filter(Boolean);
  return parts.length >= 2 ? decodeURIComponent(parts[1]) : "";
}

function formatTimestamp(ms) {
  if (ms === null || ms === undefined) return "—";
  const n = Number(ms);
  if (!Number.isFinite(n)) return "—";
  const d = new Date(n);
  if (Number.isNaN(d.getTime())) return "—";
  return d.toLocaleString();
}

function formatResponseTime(ms) {
  if (ms === null || ms === undefined) return "—";
  return `${ms} ms`;
}

function statusClass(status) {
  switch ((status || "").toLowerCase()) {
    case "up":
      return "up";
    case "down":
      return "down";
    case "degraded":
      return "degraded";
    case "paused":
      return "paused";
    default:
      return "unknown";
  }
}

// ── WS footer indicator ──────────────────────────────
function setWsStatus(state) {
  const el = qs(".sp-footer__ws");
  const label = id("footer-ws-label");
  if (!el) return;
  el.className = "sp-footer__ws";
  if (state === "connected") {
    el.classList.add("ws--connected");
    if (label) label.textContent = "Live";
  } else if (state === "disconnected") {
    el.classList.add("ws--disconnected");
    if (label) label.textContent = "Disconnected";
  } else {
    if (label) label.textContent = "Connecting…";
  }
}

// ── Error/skeleton helpers ────────────────────────────
function showError(message) {
  const skel = id("status-skeleton");
  if (skel) skel.style.display = "none";
  const box = id("error-box");
  const txt = id("error-text");
  if (!box) return;
  if (txt) txt.textContent = message || "Unknown error";
  box.classList.remove("hidden");
}

function hideError() {
  const box = id("error-box");
  if (box) box.classList.add("hidden");
}

function hideSkeleton() {
  const skel = id("status-skeleton");
  if (skel) skel.style.display = "none";
}

// ── Uptime bar ────────────────────────────────────────
function updateUptimeFill(status) {
  const fill = id("uptime-fill");
  if (!fill) return;
  const colors = {
    up: "linear-gradient(90deg, var(--up), #00bcd4)",
    down: "linear-gradient(90deg, var(--down), #c62828)",
    degraded: "linear-gradient(90deg, var(--degraded), #f57f17)",
    paused: "linear-gradient(90deg, var(--paused), #546e7a)",
    unknown: "rgba(255,255,255,.1)",
  };
  fill.style.background = colors[statusClass(status)] || colors.unknown;
}

// ── Render ────────────────────────────────────────────
function renderStatus(view) {
  statusState.currentView = view;

  const monitor = view?.monitor || {};
  const lc = view?.latest_check || null;
  const inc = view?.open_incident || null;

  document.title = `${monitor.name || "Monitor"} · PulseGrid`;

  const resolved = monitor.status || lc?.status || "unknown";
  const sc = statusClass(resolved);

  if (id("monitor-name"))
    id("monitor-name").textContent = monitor.name || "Unnamed monitor";
  if (id("monitor-url")) id("monitor-url").textContent = monitor.url || "";
  if (id("monitor-meta"))
    id("monitor-meta").textContent =
      `Slug: ${monitor.slug || "—"} · Interval: ${monitor.interval_seconds ?? "—"}s`;

  const pill = id("monitor-status-pill");
  if (pill) {
    pill.textContent = resolved.toUpperCase();
    pill.className = `sp-pill sp-pill--${sc}`;
  }

  if (id("latest-check-status"))
    id("latest-check-status").textContent = lc?.status
      ? lc.status.toUpperCase()
      : "—";
  if (id("response-time"))
    id("response-time").textContent = formatResponseTime(lc?.response_time_ms);
  if (id("checked-at"))
    id("checked-at").textContent = formatTimestamp(lc?.checked_at_ms);

  updateUptimeFill(resolved);

  const incBox = id("incident-box");
  if (inc) {
    if (id("incident-message"))
      id("incident-message").textContent = inc.message || "Incident open";
    if (id("incident-started"))
      id("incident-started").textContent =
        `Started: ${formatTimestamp(inc.started_at_ms)}`;
    incBox?.classList.remove("hidden");
  } else {
    incBox?.classList.add("hidden");
  }

  hideSkeleton();
  id("hero")?.classList.remove("hidden");
  id("metrics")?.classList.remove("hidden");
}

// ── Load ──────────────────────────────────────────────
async function loadStatus() {
  hideError();
  const slug = getSlugFromPath();
  if (!slug) {
    showError("Missing monitor slug in URL.");
    return;
  }

  try {
    const res = await fetch(`/api/status/${encodeURIComponent(slug)}`);
    if (!res.ok) {
      const d = await res.json().catch(() => ({}));
      showError(d?.error || "Failed to load public status.");
      return;
    }
    const data = await res.json();
    if (!data || !data.monitor) {
      showError("Invalid response format.");
      return;
    }
    renderStatus(data);
  } catch (err) {
    console.error("[PulseGrid Status] loadStatus failed:", err);
    showError("Network error while loading status.");
  }
}

// ── WS merge helpers ──────────────────────────────────
function mergeMonitorUpdate(data) {
  const pv = statusState.currentView || {};
  const pm = pv.monitor || {};
  const pl = pv.latest_check || null;
  const hasCheck =
    data.response_time_ms !== undefined || data.checked_at_ms !== undefined;
  const nextLatest = hasCheck
    ? {
        status: data.status ?? pl?.status ?? pm.status,
        response_time_ms:
          data.response_time_ms !== undefined
            ? data.response_time_ms
            : pl?.response_time_ms,
        checked_at_ms:
          data.checked_at_ms !== undefined
            ? data.checked_at_ms
            : pl?.checked_at_ms,
      }
    : pl;
  return {
    monitor: {
      ...pm,
      id: data.id ?? pm.id,
      name: data.name ?? pm.name,
      slug: data.slug ?? pm.slug,
      url: data.url ?? pm.url,
      interval_seconds: data.interval_seconds ?? pm.interval_seconds,
      status: data.status ?? pm.status,
      created_at_ms: data.created_at_ms ?? pm.created_at_ms,
      updated_at_ms: data.updated_at_ms ?? pm.updated_at_ms,
    },
    latest_check: nextLatest,
    open_incident: pv.open_incident || null,
  };
}

function mergeCheckRecorded(data) {
  const pv = statusState.currentView || {};
  const pm = pv.monitor || {};
  return {
    monitor: { ...pm, status: data.status ?? pm.status },
    latest_check: {
      status: data.status,
      response_time_ms: data.response_time_ms,
      checked_at_ms: data.checked_at_ms,
    },
    open_incident: pv.open_incident || null,
  };
}

function mergeIncidentOpened(data) {
  const pv = statusState.currentView || {};
  return {
    monitor: pv.monitor || {},
    latest_check: pv.latest_check || null,
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
  const pv = statusState.currentView || {};
  return {
    monitor: pv.monitor || {},
    latest_check: pv.latest_check || null,
    open_incident: null,
  };
}

// ── WS ────────────────────────────────────────────────
function wsUrl() {
  const p = window.location.protocol === "https:" ? "wss:" : "ws:";
  return `${p}//${window.location.host}/ws`;
}

function handleWsMessage(raw) {
  let msg;
  try {
    msg = JSON.parse(raw);
  } catch {
    return;
  }
  if (!msg || typeof msg !== "object") return;
  if (
    msg.type === "ws.connected" ||
    msg.type === "monitor.subscribed" ||
    msg.type === "pong"
  )
    return;

  const currentSlug = getSlugFromPath();
  const currentId = statusState.currentView?.monitor?.id || null;

  if (msg.type === "monitor.updated" && msg.data) {
    if (msg.data.slug !== currentSlug) return;
    renderStatus(mergeMonitorUpdate(msg.data));
    hideError();
    return;
  }
  if (msg.type === "check.recorded" && msg.data) {
    if (!currentId || msg.data.monitor_id !== currentId) return;
    renderStatus(mergeCheckRecorded(msg.data));
    hideError();
    return;
  }
  if (msg.type === "incident.opened" && msg.data) {
    if (!currentId || msg.data.monitor_id !== currentId) return;
    renderStatus(mergeIncidentOpened(msg.data));
    hideError();
    return;
  }
  if (msg.type === "incident.resolved" && msg.data) {
    if (!currentId || msg.data.monitor_id !== currentId) return;
    renderStatus(mergeIncidentResolved());
    hideError();
  }
}

function scheduleReconnect() {
  if (statusState.reconnectTimer) return;
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
  )
    return;
  setWsStatus("connecting");
  try {
    const socket = new WebSocket(wsUrl());
    statusState.ws = socket;

    socket.addEventListener("open", () => {
      setWsStatus("connected");
      const slug = getSlugFromPath();
      try {
        socket.send(
          JSON.stringify({ type: "monitor.subscribe", payload: { slug } }),
        );
      } catch {
        /* ignore */
      }
    });
    socket.addEventListener("message", (e) => handleWsMessage(e.data));
    socket.addEventListener("close", () => {
      statusState.ws = null;
      setWsStatus("disconnected");
      scheduleReconnect();
    });
    socket.addEventListener("error", () => {
      if (statusState.ws) statusState.ws.close();
    });
  } catch {
    setWsStatus("disconnected");
    scheduleReconnect();
  }
}

document.addEventListener("DOMContentLoaded", () => {
  loadStatus();
  connectWebSocket();
});
