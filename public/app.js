const API = {
  base: "",
  monitors: "/api/monitors",
  summary: "/api/status/summary",
  meta: "/_meta",
};

const state = {
  monitors: [],
};

function qs(selector) {
  return document.querySelector(selector);
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

async function loadSummary() {
  const summary = qs("#summary");
  if (!summary) return;

  try {
    const data = await fetchJson(API.summary);
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
  } catch (e) {
    summary.textContent = e.message || "Error loading summary";
  }
}

function renderMonitor(m) {
  const status = (m.status || "unknown").toLowerCase();
  const slugPart = m.slug
    ? `<div class="meta" style="margin-top: 6px;">Slug: <a href="/status/${encodeURIComponent(m.slug)}" style="color:#93c5fd;text-decoration:none;">${m.slug}</a></div>`
    : "";

  return `
    <div class="card">
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

async function init() {
  await loadMeta();
  await loadSummary();
  await loadMonitors();
}

document.addEventListener("DOMContentLoaded", () => {
  init();
  setInterval(loadMonitors, 5000);
  setInterval(loadSummary, 5000);
});
