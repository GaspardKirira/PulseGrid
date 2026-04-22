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
  return `${protocol}//${window.location.host}/ws`;
}

let modalResolve = null;

function openConfirmModal({ title, message }) {
  const modal = qs("#confirm-modal");
  const titleEl = qs("#modal-title");
  const messageEl = qs("#modal-message");

  if (!modal || !titleEl || !messageEl) return Promise.resolve(false);

  titleEl.textContent = title;
  messageEl.textContent = message;

  modal.classList.remove("hidden");

  return new Promise((resolve) => {
    modalResolve = resolve;
  });
}

function closeConfirmModal(result) {
  const modal = qs("#confirm-modal");
  if (!modal) return;

  modal.classList.add("hidden");

  if (modalResolve) {
    modalResolve(result);
    modalResolve = null;
  }
}

async function fetchJson(url, opts = {}) {
  const res = await fetch(API.base + url, {
    headers: {
      "Content-Type": "application/json",
      ...(opts.headers || {}),
    },
    ...opts,
  });

  const rawText = await res.text();
  let data = null;

  try {
    data = rawText ? JSON.parse(rawText) : null;
  } catch {
    data = null;
  }

  if (!res.ok) {
    throw new Error(
      data?.error || data?.message || rawText || `HTTP ${res.status}`,
    );
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
  const isPaused = status === "paused";

  const href = monitor.slug
    ? `/status/${encodeURIComponent(monitor.slug)}`
    : "#";

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
    <article class="card monitor-card" data-monitor-id="${escapeHtml(
      monitor.id || "",
    )}">
      <a
        class="card-link-area"
        href="${href}"
        aria-label="Open monitor ${escapeHtml(monitor.name || "monitor")}"
      >
        <h3>${escapeHtml(monitor.name || "Unnamed monitor")}</h3>
        <div class="meta">${escapeHtml(monitor.url || "")}</div>
        <div class="meta" style="margin-top: 6px;">Interval: ${escapeHtml(
          monitor.interval_seconds ?? "?",
        )}s</div>
        <div class="meta" style="margin-top: 6px;">Slug: ${escapeHtml(
          monitor.slug || "",
        )}</div>
        ${responseTimePart}
        ${incidentPart}
        <div class="status ${statusClass(status)}">
          ${escapeHtml(status.toUpperCase())}
        </div>
      </a>

      <div class="card-actions">
        ${
          isPaused
            ? `<button class="btn btn-small btn-success" type="button" data-action="resume" data-monitor-id="${escapeHtml(
                monitor.id || "",
              )}">Resume</button>`
            : `<button class="btn btn-small btn-warning" type="button" data-action="pause" data-monitor-id="${escapeHtml(
                monitor.id || "",
              )}">Pause</button>`
        }

        <button
          class="btn btn-small btn-danger"
          type="button"
          data-action="delete"
          data-monitor-id="${escapeHtml(monitor.id || "")}"
          data-monitor-name="${escapeHtml(monitor.name || "this monitor")}"
        >
          Delete
        </button>
      </div>
    </article>
  `;
}

function initModal() {
  const cancel = qs("#modal-cancel");
  const confirm = qs("#modal-confirm");
  const overlay = qs(".modal-overlay");

  cancel?.addEventListener("click", () => closeConfirmModal(false));
  confirm?.addEventListener("click", () => closeConfirmModal(true));
  overlay?.addEventListener("click", () => closeConfirmModal(false));

  document.addEventListener("keydown", (e) => {
    if (e.key === "Escape") {
      closeConfirmModal(false);
    }
  });
}

async function pauseMonitor(monitorId) {
  await fetchJson(`${API.monitors}/${encodeURIComponent(monitorId)}/pause`, {
    method: "POST",
  });

  await loadMonitors();
  await loadSummary();
}

async function resumeMonitor(monitorId) {
  await fetchJson(`${API.monitors}/${encodeURIComponent(monitorId)}/resume`, {
    method: "POST",
  });

  await loadMonitors();
  await loadSummary();
}

async function deleteMonitor(monitorId) {
  await fetchJson(`${API.monitors}/${encodeURIComponent(monitorId)}`, {
    method: "DELETE",
  });

  state.monitors = state.monitors.filter(
    (item) => String(item.id) !== String(monitorId),
  );
  renderMonitors();
  await loadSummary();
}

async function handleGridClick(event) {
  const button = event.target.closest("button[data-action]");
  if (!button) {
    return;
  }

  const action = button.dataset.action;
  const monitorId = button.dataset.monitorId;
  const monitorName = button.dataset.monitorName || "this monitor";

  if (!action || !monitorId) {
    return;
  }

  try {
    if (action === "pause") {
      await pauseMonitor(monitorId);
      return;
    }

    if (action === "resume") {
      await resumeMonitor(monitorId);
      return;
    }

    if (action === "delete") {
      const confirmed = await openConfirmModal({
        title: "Delete monitor",
        message: `Delete ${monitorName}? This action cannot be undone.`,
      });

      if (!confirmed) {
        return;
      }

      await deleteMonitor(monitorId);
    }
  } catch (e) {
    alert(e.message || "Action failed");
  }
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
    void loadSummary();
    return;
  }

  if (msg.type === "check.recorded") {
    void loadSummary();
    return;
  }

  if (msg.type === "incident.opened" || msg.type === "incident.resolved") {
    void loadSummary();
    return;
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
      await loadSummary();
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

  initModal();

  const form = qs("#create-monitor-form");
  if (form) {
    form.addEventListener("submit", handleCreateMonitor);
  }

  const grid = qs("#grid");
  if (grid) {
    grid.addEventListener("click", handleGridClick);
  }
}

document.addEventListener("DOMContentLoaded", () => {
  init();
});
