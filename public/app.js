const API = {
  base: "",
  monitors: "/api/monitors",
  summary: "/api/status/summary",
  meta: "/_meta",
};

const WS = {
  endpoint: "/",
  reconnectDelayMs: 3000,
};

const state = {
  monitors: [],
  ws: null,
  reconnectTimer: null,
};

function qs(selector) {
  return document.querySelector(selector);
}

function wsUrl() {
  const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
  const host = window.location.hostname;
  return `${protocol}//${host}:9090${WS.endpoint}`;
}

async function fetchJson(url, opts = {}) {
  const res = await fetch(API.base + url, {
    headers: { "Content-Type": "application/json" },
    ...opts,
  });

  const data = await res.json();
  if (!res.ok) {
    throw new Error(data?.error || "Request failed");
  }

  return data;
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

async function loadMeta() {
  const badge = qs("#meta-version");
  if (!badge) return;

  try {
    const data = await fetchJson(API.meta);
    badge.textContent = "v" + (data.version || "?");
  } catch {
    badge.textContent = "v?";
  }
}

function renderSummary(data) {
  const summary = qs("#summary");
  if (!summary) return;

  summary.textContent =
    "Total: " +
    (data.total_monitors ?? 0) +
    " • Up: " +
    (data.up_monitors ?? 0) +
    " • Down: " +
    (data.down_monitors ?? 0) +
    " • Degraded: " +
    (data.degraded_monitors ?? 0) +
    " • Paused: " +
    (data.paused_monitors ?? 0) +
    " • Open incidents: " +
    (data.open_incidents ?? 0);
}

async function loadSummary() {
  try {
    const data = await fetchJson(API.summary);
    renderSummary(data);
  } catch (e) {
    const summary = qs("#summary");
    if (summary) {
      summary.textContent = e.message || "Error loading summary";
    }
  }
}

function renderMonitor(m) {
  const status = (m.status || "unknown").toLowerCase();
  const slugPart = m.slug
    ? `<div class="meta" style="margin-top: 6px;">Slug: <a href="/status/${encodeURIComponent(m.slug)}" style="color:#93c5fd;text-decoration:none;">${m.slug}</a></div>`
    : "";

  return `
    <div class="card" data-monitor-id="${m.id || ""}">
      <h3>${m.name || "Unnamed monitor"}</h3>
      <div class="meta">${m.url || ""}</div>
      <div class="meta" style="margin-top: 6px;">Interval: ${m.interval_seconds ?? "?"}s</div>
      ${slugPart}
      <div class="status ${statusClass(status)}">
        ${status.toUpperCase()}
      </div>
    </div>
  `;
}

function renderMonitors() {
  const grid = qs("#grid");
  if (!grid) return;

  if (!state.monitors.length) {
    grid.innerHTML = "<p class='empty'>No monitors yet.</p>";
    return;
  }

  grid.innerHTML = state.monitors.map(renderMonitor).join("");
}

async function loadMonitors() {
  try {
    const data = await fetchJson(API.monitors);

    if (!data.ok) {
      throw new Error("Failed to load monitors");
    }

    state.monitors = Array.isArray(data.data) ? data.data : [];
    renderMonitors();
  } catch (e) {
    const grid = qs("#grid");
    if (grid) {
      grid.innerHTML = `<p class="empty">${e.message || "Error loading monitors"}</p>`;
    }
  }
}

function upsertMonitor(monitor) {
  const index = state.monitors.findIndex((item) => item.id === monitor.id);

  if (index === -1) {
    state.monitors.push(monitor);
  } else {
    state.monitors[index] = {
      ...state.monitors[index],
      ...monitor,
    };
  }

  renderMonitors();
}

function updateSummaryFromMonitors() {
  const totals = {
    total_monitors: state.monitors.length,
    up_monitors: 0,
    down_monitors: 0,
    degraded_monitors: 0,
    paused_monitors: 0,
    open_incidents: 0,
  };

  for (const monitor of state.monitors) {
    switch ((monitor.status || "").toLowerCase()) {
      case "up":
        totals.up_monitors += 1;
        break;
      case "down":
        totals.down_monitors += 1;
        break;
      case "degraded":
        totals.degraded_monitors += 1;
        break;
      case "paused":
        totals.paused_monitors += 1;
        break;
      default:
        break;
    }
  }

  renderSummary(totals);
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

  if (msg.type === "ws.connected") {
    try {
      state.ws?.send(
        JSON.stringify({
          type: "status.subscribe",
          payload: {},
        }),
      );
    } catch {}
    return;
  }

  if (msg.type === "status.subscribed" || msg.type === "pong") {
    return;
  }

  if (
    (msg.type === "monitor.updated" || msg.type === "monitor.created") &&
    msg.data
  ) {
    upsertMonitor(msg.data);
    updateSummaryFromMonitors();
    return;
  }

  if (
    msg.type === "check.recorded" ||
    msg.type === "incident.opened" ||
    msg.type === "incident.resolved"
  ) {
    void loadSummary();
    void loadMonitors();
  }
}

function scheduleReconnect() {
  if (state.reconnectTimer) {
    return;
  }

  state.reconnectTimer = window.setTimeout(() => {
    state.reconnectTimer = null;
    connectWebSocket();
  }, WS.reconnectDelayMs);
}

function connectWebSocket() {
  if (
    state.ws &&
    (state.ws.readyState === WebSocket.OPEN ||
      state.ws.readyState === WebSocket.CONNECTING)
  ) {
    return;
  }

  try {
    const socket = new WebSocket(wsUrl());
    state.ws = socket;

    socket.addEventListener("open", () => {
      console.log("[PulseGrid] WS connected");
    });

    socket.addEventListener("message", (event) => {
      handleWsMessage(event.data);
    });

    socket.addEventListener("close", () => {
      console.log("[PulseGrid] WS disconnected");
      state.ws = null;
      scheduleReconnect();
    });

    socket.addEventListener("error", () => {
      socket.close();
    });
  } catch {
    scheduleReconnect();
  }
}

async function init() {
  await loadMeta();
  await loadSummary();
  await loadMonitors();
  connectWebSocket();
}

document.addEventListener("DOMContentLoaded", () => {
  init();
});

function showFormMessage(message, isError = false) {
  const box = qs("#form-message");
  if (!box) return;

  box.textContent = message;
  box.style.color = isError ? "#ef4444" : "#22c55e";
}

async function handleCreateMonitor(event) {
  event.preventDefault();

  const name = qs("#monitor-name").value.trim();
  const slug = qs("#monitor-slug").value.trim();
  const url = qs("#monitor-url").value.trim();
  const interval = parseInt(qs("#monitor-interval").value, 10) || 30;

  try {
    const res = await fetch(API.monitors, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        name,
        slug,
        url,
        interval_seconds: interval,
      }),
    });

    const data = await res.json();

    if (!res.ok) {
      throw new Error(data?.error || "Failed to create monitor");
    }

    showFormMessage("Monitor created successfully");

    // reset form
    qs("#create-monitor-form").reset();

    // 👉 update UI immédiatement
    if (data) {
      upsertMonitor(data);
      updateSummaryFromMonitors();
    }
  } catch (e) {
    showFormMessage(e.message, true);
  }
}

document.addEventListener("DOMContentLoaded", () => {
  const form = qs("#create-monitor-form");
  if (form) {
    form.addEventListener("submit", handleCreateMonitor);
  }
});
