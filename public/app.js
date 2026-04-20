// PulseGrid frontend (minimal, clean, production-style)
// Uses plain fetch + optional WebSocket for live updates

const API = {
  base: "",
  monitors: "/api/monitors",
  health: "/api/health",
  status: "/api/status",
};

const state = {
  monitors: [],
  connected: false,
};

// -----------------------------------------------------
// Utils
// -----------------------------------------------------

function qs(selector) {
  return document.querySelector(selector);
}

function create(tag, attrs = {}, children = []) {
  const el = document.createElement(tag);

  for (const [k, v] of Object.entries(attrs)) {
    if (k === "class") el.className = v;
    else if (k === "text") el.textContent = v;
    else el.setAttribute(k, v);
  }

  for (const c of children) {
    el.appendChild(c);
  }

  return el;
}

function statusClass(status) {
  switch (status) {
    case "up":
      return "status-up";
    case "down":
      return "status-down";
    case "degraded":
      return "status-degraded";
    default:
      return "status-unknown";
  }
}

// -----------------------------------------------------
// API
// -----------------------------------------------------

async function fetchJson(url, opts = {}) {
  const res = await fetch(API.base + url, {
    headers: { "Content-Type": "application/json" },
    ...opts,
  });

  if (!res.ok) {
    const text = await res.text();
    throw new Error(text || "Request failed");
  }

  return res.json();
}

async function loadMonitors() {
  try {
    const data = await fetchJson(API.monitors);
    state.monitors = data.monitors || data || [];
    renderMonitors();
  } catch (e) {
    showError(e.message);
  }
}

async function createMonitor(payload) {
  try {
    await fetchJson(API.monitors, {
      method: "POST",
      body: JSON.stringify(payload),
    });

    await loadMonitors();
    showSuccess("Monitor created");
  } catch (e) {
    showError(e.message);
  }
}

// -----------------------------------------------------
// Rendering
// -----------------------------------------------------

function renderMonitors() {
  const container = qs("#monitors");
  if (!container) return;

  container.innerHTML = "";

  if (!state.monitors.length) {
    container.appendChild(
      create("div", { class: "empty-state", text: "No monitors yet." }),
    );
    return;
  }

  for (const m of state.monitors) {
    const card = create("div", { class: "card" });

    const header = create("div", { class: "row-between" }, [
      create("div", {}, [
        create("h3", { class: "monitor-name", text: m.name || "Unnamed" }),
        create("div", { class: "monitor-url", text: m.url }),
      ]),
      create("span", {
        class: `status-pill ${statusClass(m.status)}`,
        text: m.status || "unknown",
      }),
    ]);

    const meta = create("div", {
      class: "monitor-meta",
      text: `interval: ${m.interval || "?"}s`,
    });

    card.appendChild(header);
    card.appendChild(meta);

    container.appendChild(card);
  }
}

// -----------------------------------------------------
// Form
// -----------------------------------------------------

function setupForm() {
  const form = qs("#create-form");
  if (!form) return;

  form.addEventListener("submit", (e) => {
    e.preventDefault();

    const name = form.querySelector("input[name=name]").value.trim();
    const url = form.querySelector("input[name=url]").value.trim();
    const interval = parseInt(
      form.querySelector("input[name=interval]").value,
      10,
    );

    if (!url) {
      showError("URL is required");
      return;
    }

    createMonitor({
      name,
      url,
      interval: interval || 30,
    });

    form.reset();
  });
}

// -----------------------------------------------------
// Notifications
// -----------------------------------------------------

function showError(msg) {
  const box = qs("#messages");
  if (!box) return;

  box.innerHTML = "";
  box.appendChild(create("div", { class: "error-box", text: msg }));
}

function showSuccess(msg) {
  const box = qs("#messages");
  if (!box) return;

  box.innerHTML = "";
  box.appendChild(create("div", { class: "success-box", text: msg }));
}

// -----------------------------------------------------
// WebSocket (optional real-time)
// -----------------------------------------------------

function connectWS() {
  if (!("WebSocket" in window)) return;

  try {
    const protocol = location.protocol === "https:" ? "wss://" : "ws://";
    const ws = new WebSocket(protocol + location.host + "/ws/status");

    ws.onopen = () => {
      state.connected = true;
      console.log("WS connected");
    };

    ws.onclose = () => {
      state.connected = false;
      console.log("WS disconnected, retrying...");
      setTimeout(connectWS, 3000);
    };

    ws.onmessage = (ev) => {
      try {
        const data = JSON.parse(ev.data);

        if (data.type === "status_update") {
          const m = state.monitors.find((x) => x.id === data.id);
          if (m) {
            m.status = data.status;
            renderMonitors();
          }
        }
      } catch (_) {}
    };
  } catch (e) {
    console.warn("WS error", e);
  }
}

// -----------------------------------------------------
// Init
// -----------------------------------------------------

function init() {
  setupForm();
  loadMonitors();
  connectWS();
}

document.addEventListener("DOMContentLoaded", init);
