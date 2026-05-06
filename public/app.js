const API = {
  base: "",
  monitors: "/api/monitors",
  summary: "/api/status/summary",
  meta: "/_meta",
};

const WS = {
  reconnectDelayMs: 5000,
  maxReconnectDelayMs: 30000,
};

let reconnectAttempts = 0;

const state = {
  monitors: [],
  ws: null,
  reconnectTimer: null,
  loaded: false,
};

function qs(selector) {
  return document.querySelector(selector);
}

function wsUrl() {
  const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
  return `${protocol}//${window.location.host}/ws`;
}

// ── WS Status indicator ──────────────────────────────
function setWsStatus(state) {
  const el = qs("#ws-status");
  if (!el) return;
  const label = el.querySelector(".ws-status__label");
  el.className = "ws-status";
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

// ── Modal ────────────────────────────────────────────
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

// ── HTTP helpers ─────────────────────────────────────
async function fetchJson(url, opts = {}) {
  const res = await fetch(API.base + url, {
    headers: { "Content-Type": "application/json", ...(opts.headers || {}) },
    ...opts,
  });
  const rawText = await res.text();
  let data = null;
  try {
    data = rawText ? JSON.parse(rawText) : null;
  } catch {
    data = null;
  }
  if (!res.ok)
    throw new Error(
      data?.error || data?.message || rawText || `HTTP ${res.status}`,
    );
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

// ── Normalize ────────────────────────────────────────
function normalizeMonitor(input) {
  if (!input || typeof input !== "object") return null;
  if (input.monitor && typeof input.monitor === "object") {
    return {
      ...input.monitor,
      latest_check: input.latest_check ?? null,
      open_incident: input.open_incident ?? null,
      status: input.monitor.status || input.latest_check?.status || "unknown",
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
  if (Array.isArray(payload))
    return payload.map(normalizeMonitor).filter(Boolean);
  if (payload && Array.isArray(payload.data))
    return payload.data.map(normalizeMonitor).filter(Boolean);
  if (payload && Array.isArray(payload.monitors))
    return payload.monitors.map(normalizeMonitor).filter(Boolean);
  return [];
}

// ── Form messages ─────────────────────────────────────
function showFormMessage(message, isError = false) {
  const box = qs("#form-message");
  if (!box) return;
  box.textContent = message || "";
  box.style.color = isError ? "#ff1744" : "#00e676";
}

// ── Meta / version ────────────────────────────────────
async function loadMeta() {
  const badge = qs("#meta-version");
  if (!badge) return;
  try {
    const data = await fetchJson(API.meta);
    badge.innerHTML = `<span class="version-chip__dot"></span>v${data?.version || "?"}`;
  } catch {
    badge.innerHTML = `<span class="version-chip__dot"></span>v?`;
  }
}

// ── Summary ───────────────────────────────────────────
function renderSummary(data) {
  const set = (id, val) => {
    const el = document.getElementById(id);
    if (el) el.textContent = val ?? 0;
  };
  set("summary-total", data?.total_monitors);
  set("summary-up", data?.up_monitors);
  set("summary-down", data?.down_monitors);
  set("summary-degraded", data?.degraded_monitors);
  set("summary-paused", data?.paused_monitors);
  set("summary-incidents", data?.open_incidents);
}

async function loadSummary() {
  try {
    const data = await fetchJson(API.summary);
    renderSummary(data);
  } catch {
    /* silent */
  }
}

// ── Render monitor card ──────────────────────────────
function renderMonitor(monitor) {
  const sc = statusClass(monitor.status);
  const href = monitor.slug
    ? `/status/${encodeURIComponent(monitor.slug)}`
    : "#";
  const lc = monitor.latest_check;
  const inc = monitor.open_incident;
  const isPaused = sc === "paused";

  const rtag =
    lc?.response_time_ms != null
      ? `<span class="mcard__tag">⚡ ${escapeHtml(lc.response_time_ms)} ms</span>`
      : "";
  const slug = monitor.slug
    ? `<span class="mcard__tag"># ${escapeHtml(monitor.slug)}</span>`
    : "";
  const interval =
    monitor.interval_seconds != null
      ? `<span class="mcard__tag">⏱ ${escapeHtml(monitor.interval_seconds)}s</span>`
      : "";
  const incidentBlock = inc?.message
    ? `<div class="mcard__incident">
         <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>
         ${escapeHtml(inc.message)}
       </div>`
    : "";
  const pauseBtn = isPaused
    ? `<button class="btn btn--sm btn--resume" type="button" data-action="resume" data-monitor-id="${escapeHtml(monitor.id || "")}">Resume</button>`
    : `<button class="btn btn--sm btn--pause"  type="button" data-action="pause"  data-monitor-id="${escapeHtml(monitor.id || "")}">Pause</button>`;

  return `
    <article class="mcard mcard--${sc}" data-monitor-id="${escapeHtml(monitor.id || "")}">
      <a class="mcard__link" href="${href}" aria-label="View ${escapeHtml(monitor.name || "monitor")}">
        <div class="mcard__top">
          <h3 class="mcard__name">${escapeHtml(monitor.name || "Unnamed monitor")}</h3>
          <div class="status-badge status-badge--${sc}">
            <span class="status-badge__dot"></span>
            ${escapeHtml((monitor.status || "unknown").toUpperCase())}
          </div>
        </div>
        <div class="mcard__url">${escapeHtml(monitor.url || "")}</div>
        <div class="mcard__meta">${slug}${interval}${rtag}</div>
      </a>
      ${incidentBlock}
      <div class="mcard__actions">
        ${pauseBtn}
        <button
          class="btn btn--sm btn--del"
          type="button"
          data-action="delete"
          data-monitor-id="${escapeHtml(monitor.id || "")}"
          data-monitor-name="${escapeHtml(monitor.name || "this monitor")}">
          Delete
        </button>
      </div>
    </article>`;
}

// ── Render grid ───────────────────────────────────────
function renderMonitors() {
  const skeleton = qs("#grid-skeleton");
  if (skeleton) skeleton.style.display = "none";

  const grid = qs("#grid");
  if (!grid) return;

  if (!state.monitors.length) {
    grid.innerHTML = `
      <div class="empty-state">
        <svg width="40" height="40" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
          <rect x="2" y="3" width="20" height="14" rx="2"/><line x1="8" y1="21" x2="16" y2="21"/><line x1="12" y1="17" x2="12" y2="21"/>
        </svg>
        <p>No monitors yet — add one to get started.</p>
      </div>`;
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
    const skeleton = qs("#grid-skeleton");
    if (skeleton) skeleton.style.display = "none";
    const grid = qs("#grid");
    if (grid)
      grid.innerHTML = `<div class="empty-state"><p>${escapeHtml(e.message || "Error loading monitors")}</p></div>`;
  }
}

// ── Upsert monitor ────────────────────────────────────
function upsertMonitor(monitor) {
  const normalized = normalizeMonitor(monitor);
  if (!normalized) return;
  const index = state.monitors.findIndex((m) => m.id === normalized.id);
  if (index === -1) state.monitors.unshift(normalized);
  else state.monitors[index] = { ...state.monitors[index], ...normalized };
  renderMonitors();
}

// ── Actions ───────────────────────────────────────────
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
    (m) => String(m.id) !== String(monitorId),
  );
  renderMonitors();
  await loadSummary();
}

async function handleGridClick(event) {
  const button = event.target.closest("button[data-action]");
  if (!button) return;
  const { action, monitorId, monitorName = "this monitor" } = button.dataset;
  if (!action || !monitorId) return;
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
      const ok = await openConfirmModal({
        title: "Delete monitor",
        message: `Delete "${monitorName}"? This cannot be undone.`,
      });
      if (ok) await deleteMonitor(monitorId);
    }
  } catch (e) {
    alert(e.message || "Action failed");
  }
}

// ── WebSocket ─────────────────────────────────────────
function handleWsMessage(raw) {
  let msg;
  try {
    msg = JSON.parse(raw);
  } catch {
    return;
  }
  if (!msg || typeof msg !== "object") return;

  if (msg.type === "ws.connected") {
    try {
      state.ws?.send(JSON.stringify({ type: "status.subscribe", payload: {} }));
    } catch {
      /* ignore */
    }
    return;
  }
  if (msg.type === "status.subscribed" || msg.type === "pong") return;
  if (
    (msg.type === "monitor.updated" || msg.type === "monitor.created") &&
    msg.data
  ) {
    upsertMonitor(msg.data);
    return;
  }
  if (msg.type === "check.recorded") {
    loadSummary();
    return;
  }
  if (msg.type === "incident.opened" || msg.type === "incident.resolved") {
    loadMonitors();
    loadSummary();
  }
}

function scheduleReconnect() {
  if (state.reconnectTimer) return;
  const delay = Math.min(
    WS.reconnectDelayMs * Math.pow(2, reconnectAttempts),
    WS.maxReconnectDelayMs,
  );
  state.reconnectTimer = window.setTimeout(() => {
    state.reconnectTimer = null;
    reconnectAttempts++;
    connectWebSocket();
  }, delay);
}

function connectWebSocket() {
  if (
    state.ws &&
    (state.ws.readyState === WebSocket.OPEN ||
      state.ws.readyState === WebSocket.CONNECTING)
  )
    return;
  setWsStatus("connecting");
  try {
    const socket = new WebSocket(wsUrl());
    state.ws = socket;
    let pingInterval = null;

    socket.addEventListener("open", () => {
      reconnectAttempts = 0;
      setWsStatus("connected");
      pingInterval = setInterval(() => {
        if (socket.readyState === WebSocket.OPEN)
          socket.send(JSON.stringify({ type: "ping" }));
      }, 30000);
    });
    socket.addEventListener("message", (e) => handleWsMessage(e.data));
    socket.addEventListener("close", () => {
      if (pingInterval) {
        clearInterval(pingInterval);
        pingInterval = null;
      }
      state.ws = null;
      setWsStatus("disconnected");
      scheduleReconnect();
    });
    socket.addEventListener("error", () => socket.close());
  } catch {
    setWsStatus("disconnected");
    scheduleReconnect();
  }
}

// ── Create form ───────────────────────────────────────
async function handleCreateMonitor(event) {
  event.preventDefault();
  const nameInput = qs("#monitor-name");
  const slugInput = qs("#monitor-slug");
  const urlInput = qs("#monitor-url");
  const intervalInput = qs("#monitor-interval");
  const form = qs("#create-monitor-form");
  if (!nameInput || !slugInput || !urlInput || !intervalInput || !form) return;

  const name = nameInput.value.trim();
  const slug = slugInput.value.trim();
  const url = urlInput.value.trim();
  const interval = parseInt(intervalInput.value, 10) || 60;

  if (!name || !slug || !url) {
    showFormMessage("Please fill in all required fields", true);
    return;
  }

  showFormMessage("Creating monitor…", false);
  try {
    const data = await fetchJson(API.monitors, {
      method: "POST",
      body: JSON.stringify({ name, slug, url, interval_seconds: interval }),
    });
    const created = normalizeMonitor(data?.data) || normalizeMonitor(data);
    showFormMessage("Monitor created successfully");
    form.reset();
    if (created) {
      upsertMonitor(created);
      await loadSummary();
    } else {
      await loadMonitors();
      await loadSummary();
    }
  } catch (e) {
    showFormMessage(e.message || "Failed to create monitor", true);
  }
}

// ── Init ──────────────────────────────────────────────
function initModal() {
  qs("#modal-cancel")?.addEventListener("click", () =>
    closeConfirmModal(false),
  );
  qs("#modal-confirm")?.addEventListener("click", () =>
    closeConfirmModal(true),
  );
  qs(".modal__overlay")?.addEventListener("click", () =>
    closeConfirmModal(false),
  );
  document.addEventListener("keydown", (e) => {
    if (e.key === "Escape") closeConfirmModal(false);
  });
}

async function init() {
  await loadMeta();
  await loadSummary();
  await loadMonitors();
  connectWebSocket();
  initModal();
  qs("#create-monitor-form")?.addEventListener("submit", handleCreateMonitor);
  qs("#grid")?.addEventListener("click", handleGridClick);
}

document.addEventListener("DOMContentLoaded", () => init());
