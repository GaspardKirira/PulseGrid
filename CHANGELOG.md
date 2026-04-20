feat: initialize PulseGrid app with full architecture, migrations, and Vix integration

- bootstrap PulseGrid application using Vix runtime
- implement clean layered architecture:
  domain / application / infrastructure / presentation / app

- add core domain models:
  monitor, check, incident, shared types

- implement application services:
  monitor, check, incident, status

- define ports and repository interfaces

- add infrastructure layer:
  - SQLite repositories
  - HTTP checker + scheduler
  - WebSocket event bus
  - runtime utilities (clock, uuid, logger)

- implement HTTP controllers and WebSocket gateway

- add support layer:
  constants, errors, JSON helpers

- integrate database factory with config support

- add migration system:
  - SQL migrations (up/down)
  - FileMigrationsRunner integration
  - automatic migration on startup

- add public UI:
  status page + minimal frontend

- add test suite:
  domain + application tests

- configure CMake with Vix:
  - minimal setup
  - static linking support
  - no complex boilerplate

- add environment configuration (.env)

This project serves as a real-world reference implementation of Vix.cpp
applied to a production-style system.
