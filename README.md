# PulseGrid

Minimal Vix.cpp application.

## Quick start

```bash
cd PulseGrid
cp .env.example .env
vix build
vix run
```

Then open:

```
http://localhost:8080
```

## Dependencies

This project uses a `vix.json` manifest.

Workflow:

- `vix add <pkg>` → add dependency
- `vix install` → install dependencies
- `vix.lock` → ensures reproducible builds

Example:

```bash
vix add gk/json@^1.0.0
vix install
```

## Tasks

Run project tasks:

```bash
vix task <name>
```

Common tasks:

```bash
vix task dev
vix task test
vix task ci
```

Edit `vix.json` to customize tasks and pipelines.

## Configuration

Vix uses `.env` files for configuration.

Start by copying the example:

```bash
cp .env.example .env
```

Example:

```env
SERVER_PORT=8080
DATABASE_ENGINE=mysql
DATABASE_DEFAULT_HOST=127.0.0.1
DATABASE_DEFAULT_PORT=3306
DATABASE_DEFAULT_USER=root
DATABASE_DEFAULT_PASSWORD=
DATABASE_DEFAULT_NAME=appdb
LOGGING_ASYNC=true
WAF_MODE=basic
```

## Using configuration in code

```cpp
#include <vix.hpp>
using namespace vix;

int main()
{
  config::Config cfg{".env"};

  App app;
  app.get("/", [](Request&, Response& res) {
    res.send("Hello world");
  });

  app.run(cfg.getServerPort());
}
```

## Environment mapping

Vix maps config keys to environment variables:

- `server.port` → `SERVER_PORT`
- `database.default.host` → `DATABASE_DEFAULT_HOST`
- `database.default.name` → `DATABASE_DEFAULT_NAME`

This keeps the C++ API clean and environment-driven.

## Environment layers

You can use multiple env files:

- `.env`
- `.env.local`
- `.env.production`

Use `.env` for development and environment-specific files for deployment.
