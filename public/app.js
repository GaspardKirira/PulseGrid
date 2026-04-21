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
    headers: {
      "Content-Type": "application/json",
      ...(opts.headers || {}),
    },
    ...opts,
  });

  let data = null;

  try {
    data = await res.json();
  } catch {
    data = null;
  }

  if (!res.ok) {
    throw new Error(data?.error || data?.message || "Request failed");
  }

  return data;
}

function escapeHtml(value) {
  return String(value ?? "")
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;")
    .replaceAll("'", "&#039;");
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

function normalizeMonitor(input) {
  if (!input || typeof input !== "object") {
    return null;
  }

  if (input.monitor && typeof input.monitor === "object") {
    const latestCheck = input.latest_check ?? null;
    const openIncident = input.open_incident ?? null;

    return {
      ...input.monitor,
      latest_check: latestCheck,
      open_incident: openIncident,
      status: input.monitor.status || latestCheck?.status || "unknown",
    };
  }

  return {
    ...input,
    latest_check: input.latest_check ?? null,
    open_incident: input.open_incident ?? null,
    status: input.status || "unknown",
  };
}

function normalizeMonitorList(payload) {
  if (Array.isArray(payload)) {
    return payload.map(normalizeMonitor).filter(Boolean);
  }

  if (payload && Array.isArray(payload.data)) {
    return payload.data.map(normalizeMonitor).filter(Boolean);
  }

  if (payload && Array.isArray(payload.monitors)) {
    return payload.monitors.map(normalizeMonitor).filter(Boolean);
  }

  return [];
}

function showFormMessage(message, isError = false) {
  const box = qs("#form-message");
  if (!box) {
    return;
  }

  box.textContent = message || "";
  box.style.color = isError ? "#ef4444" : "#22c55e";
}

async function loadMeta() {
  const badge = qs("#meta-version");
  if (!badge) {
    return;
  }

  try {
    const data = await fetchJson(API.meta);
    badge.textContent = "v" + (data?.version || "?");
  } catch {
    badge.textContent = "v?";
  }
}

function renderSummary(data) {
  const total = document.getElementById("summary-total");
  const up = document.getElementById("summary-up");
  const down = document.getElementById("summary-down");
  const degraded = document.getElementById("summary-degraded");
  const paused = document.getElementById("summary-paused");
  const incidents = document.getElementById("summary-incidents");

  if (!total || !up || !down || !degraded || !paused || !incidents) {
    return;
  }

  total.textContent = data?.total_monitors ?? 0;
  up.textContent = data?.up_monitors ?? 0;
  down.textContent = data?.down_monitors ?? 0;
  degraded.textContent = data?.degraded_monitors ?? 0;
  paused.textContent = data?.paused_monitors ?? 0;
  incidents.textContent = data?.open_incidents ?? 0;
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

function renderMonitor(monitor) {
  const status = (monitor.status || "unknown").toLowerCase();
  const latestCheck = monitor.latest_check;
  const openIncident = monitor.open_incident;

  const slugPart = monitor.slug
    ? `<div class="meta" style="margin-top: 6px;">Slug: <a href="/status/${encodeURIComponent(
        monitor.slug,
      )}" style="color:#93c5fd;text-decoration:none;">${escapeHtml(monitor.slug)}</a></div>`
    : "";

  const responseTimePart =
    latestCheck && latestCheck.response_time_ms != null
      ? `<div class="meta" style="margin-top: 6px;">Response time: ${escapeHtml(
          latestCheck.response_time_ms,
        )} ms</div>`
      : "";

  const incidentPart =
    openIncident && openIncident.message
      ? `<div class="meta" style="margin-top: 6px; color:#fca5a5;">Incident: ${escapeHtml(
          openIncident.message,
        )}</div>`
      : "";

  return `
    <div class="card" data-monitor-id="${escapeHtml(monitor.id || "")}">
      <h3>${escapeHtml(monitor.name || "Unnamed monitor")}</h3>
      <div class="meta">${escapeHtml(monitor.url || "")}</div>
      <div class="meta" style="margin-top: 6px;">Interval: ${escapeHtml(
        monitor.interval_seconds ?? "?",
      )}s</div>
      ${slugPart}
      ${responseTimePart}
      ${incidentPart}
      <div class="status ${statusClass(status)}">
        ${escapeHtml(status.toUpperCase())}
      </div>
    </div>
  `;
}

function renderMonitors() {
  const grid = qs("#grid");
  if (!grid) {
    return;
  }

  if (!state.monitors.length) {
    grid.innerHTML = "<p class='empty'>No monitors yet.</p>";
    return;
  }

  grid.innerHTML = state.monitors.map(renderMonitor).join("");
}

async function loadMonitors() {
  try {
    const data = await fetchJson(API.monitors);
    state.monitors = normalizeMonitorList(data);
    renderMonitors();
  } catch (e) {
    const grid = qs("#grid");
    if (grid) {
      grid.innerHTML = `<p class="empty">${escapeHtml(
        e.message || "Error loading monitors",
      )}</p>`;
    }
  }
}

function upsertMonitor(monitor) {
  const normalized = normalizeMonitor(monitor);
  if (!normalized) {
    return;
  }

  const index = state.monitors.findIndex((item) => item.id === normalized.id);

  if (index === -1) {
    state.monitors.unshift(normalized);
  } else {
    state.monitors[index] = {
      ...state.monitors[index],
      ...normalized,
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

    if (monitor.open_incident) {
      totals.open_incidents += 1;
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
    } catch {
      // ignore
    }
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

async function handleCreateMonitor(event) {
  event.preventDefault();

  const nameInput = qs("#monitor-name");
  const slugInput = qs("#monitor-slug");
  const urlInput = qs("#monitor-url");
  const intervalInput = qs("#monitor-interval");
  const form = qs("#create-monitor-form");

  if (!nameInput || !slugInput || !urlInput || !intervalInput || !form) {
    return;
  }

  const name = nameInput.value.trim();
  const slug = slugInput.value.trim();
  const url = urlInput.value.trim();
  const interval = parseInt(intervalInput.value, 10) || 60;

  if (!name || !slug || !url) {
    showFormMessage("Please fill in all required fields", true);
    return;
  }

  showFormMessage("Creating monitor...", false);

  try {
    const data = await fetchJson(API.monitors, {
      method: "POST",
      body: JSON.stringify({
        name,
        slug,
        url,
        interval_seconds: interval,
      }),
    });

    const createdMonitor =
      normalizeMonitor(data?.data) || normalizeMonitor(data);

    showFormMessage("Monitor created successfully");

    form.reset();

    if (createdMonitor) {
      upsertMonitor(createdMonitor);
      updateSummaryFromMonitors();
    } else {
      await loadMonitors();
      await loadSummary();
    }
  } catch (e) {
    showFormMessage(e.message || "Failed to create monitor", true);
  }
}

async function init() {
  await loadMeta();
  await loadSummary();
  await loadMonitors();
  connectWebSocket();

  const form = qs("#create-monitor-form");
  if (form) {
    form.addEventListener("submit", handleCreateMonitor);
  }
}

document.addEventListener("DOMContentLoaded", () => {
  init();
});
