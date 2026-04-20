# PulseGrid

**Real-time service monitoring. Built with modern C++.**

PulseGrid is a lightweight monitoring application designed to demonstrate how to build a real-world, production-style system using **Vix.cpp**.

It focuses on clarity, reliability, and architecture, not complexity.

## What is PulseGrid?

PulseGrid lets you:

- Monitor HTTP endpoints
- Track uptime and response time
- Detect incidents automatically
- Expose a real-time status dashboard
- Stream updates via WebSocket

It is intentionally simple, but architecturally serious.

## Why this project exists

PulseGrid is not just another app.

It is a **reference implementation** showing:

- How to build a clean C++ backend
- How to structure a real application (`domain -> application -> infrastructure -> presentation`)
- How to use **Vix.cpp in production conditions**
- How to remove friction from CMake and setup

No boilerplate. No hidden magic. Just clear systems.

## Architecture

PulseGrid follows a strict layered architecture:

```text
Domain         -> Business rules (pure C++)
Application    -> Use cases / services
Infrastructure -> DB, HTTP checks, runtime
Presentation   -> HTTP API + WebSocket
App            -> Bootstrap + wiring
```

Key principles:

- Dependency inversion (ports & adapters)
- Explicit boundaries
- Testable components
- Minimal coupling

## Tech stack

- **C++20**
- **Vix.cpp runtime**
- SQLite (default)
- HTTP checks + scheduler
- WebSocket for real-time updates
- Zero external frameworks

## Project structure

```text
src/pulsegrid/
├── app/              -> bootstrap & wiring
├── domain/           -> core logic
├── application/      -> services & ports
├── infrastructure/   -> DB, events, checker
└── presentation/     -> HTTP + WS layer
```

## Getting started

### 1. Build

```bash
vix build
```

### 2. Run

```bash
vix run
```

### 3. Open

```text
http://localhost:8080
http://localhost:8080/status
```

## Tests

```bash
vix tests
```

## Database

- Default: SQLite
- Automatically initialized
- Migrations applied on startup

No manual setup required.

## Real-time updates

PulseGrid pushes updates using WebSocket:

- monitor status changes
- incidents
- live checks

## What this demonstrates

PulseGrid proves that you can:

- Build backend systems in C++ without friction
- Use modern architecture patterns cleanly
- Replace heavy stacks with a minimal runtime
- Ship something real, not just demos

## Philosophy

Build real systems. Keep them simple. Make them predictable.

## Author

Gaspard Kirira \
https://github.com/GaspardKirira

## License

MIT

