/**
 *
 *  @file DatabaseFactory.cpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 */

#include "DatabaseFactory.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include <pulsegrid/support/Constants.hpp>
#include <pulsegrid/support/Errors.hpp>
#include <vix/db/mig/FileMigrationsRunner.hpp>
#include <vix/db/pool/ConnectionPool.hpp>

namespace pulsegrid::infrastructure::db
{
  namespace
  {
    [[nodiscard]] std::string to_lower_ascii(std::string value)
    {
      std::transform(
          value.begin(),
          value.end(),
          value.begin(),
          [](unsigned char c)
          {
            return static_cast<char>(std::tolower(c));
          });

      return value;
    }
  } // namespace

  std::shared_ptr<vix::db::Database> DatabaseFactory::make(
      const vix::config::Config &config)
  {
    try
    {
      const std::string engine =
          to_lower_ascii(config.getString("database.engine", "sqlite"));

      const std::string migrations_dir = config.getString(
          "database.migrations.dir",
          std::string(pulsegrid::support::constants::default_migrations_dir));

      if (engine == "sqlite")
      {
        const std::string sqlite_path = config.getString(
            "database.sqlite.path",
            std::string(pulsegrid::support::constants::default_database_path));

        vix::db::DbConfig db_config;
        db_config.engine = vix::db::Engine::SQLite;
        db_config.sqlite.path = sqlite_path;

        db_config.sqlite.pool.min = static_cast<std::size_t>(
            std::max(1, config.getInt("database.pool.min", 1)));

        db_config.sqlite.pool.max = static_cast<std::size_t>(
            std::max(
                static_cast<int>(db_config.sqlite.pool.min),
                config.getInt("database.pool.max", 4)));

        auto database = std::make_shared<vix::db::Database>(db_config);
        apply_migrations(*database, migrations_dir);
        return database;
      }

      if (engine == "mysql")
      {
        vix::db::DbConfig db_config;
        db_config.engine = vix::db::Engine::MySQL;

        const std::string host =
            config.getString("database.default.host", "127.0.0.1");
        const int port =
            config.getInt("database.default.port", 3306);

        db_config.mysql.host = "tcp://" + host + ":" + std::to_string(port);
        db_config.mysql.user = config.getString("database.default.user", "root");
        db_config.mysql.password = config.getDbPasswordFromEnv();
        db_config.mysql.database = config.getString("database.default.name", "");

        db_config.mysql.pool.min = static_cast<std::size_t>(
            std::max(1, config.getInt("database.pool.min", 1)));

        db_config.mysql.pool.max = static_cast<std::size_t>(
            std::max(
                static_cast<int>(db_config.mysql.pool.min),
                config.getInt("database.pool.max", 8)));

        auto database = std::make_shared<vix::db::Database>(db_config);
        apply_migrations(*database, migrations_dir);
        return database;
      }

      throw pulsegrid::support::ConfigError(
          "Unsupported database.engine: " + engine);
    }
    catch (const pulsegrid::support::ConfigError &)
    {
      throw;
    }
    catch (const pulsegrid::support::PersistenceError &)
    {
      throw;
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to initialize database: ") + e.what());
    }
  }

  std::shared_ptr<vix::db::Database> DatabaseFactory::make_sqlite_default()
  {
    try
    {
      vix::db::DbConfig db_config;
      db_config.engine = vix::db::Engine::SQLite;
      db_config.sqlite.path =
          std::string(pulsegrid::support::constants::default_database_path);
      db_config.sqlite.pool.min = 1;
      db_config.sqlite.pool.max = 4;

      auto database = std::make_shared<vix::db::Database>(db_config);

      apply_migrations(
          *database,
          std::string(pulsegrid::support::constants::default_migrations_dir));

      return database;
    }
    catch (const pulsegrid::support::PersistenceError &)
    {
      throw;
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to initialize default SQLite database: ") + e.what());
    }
  }

  void DatabaseFactory::apply_migrations(
      vix::db::Database &database,
      const std::string &migrations_dir)
  {
    try
    {
      vix::db::PooledConn conn(database.pool());
      vix::db::FileMigrationsRunner runner(*conn, migrations_dir);
      runner.applyAll();
    }
    catch (const std::exception &e)
    {
      throw pulsegrid::support::PersistenceError(
          std::string("Failed to apply database migrations: ") + e.what());
    }
  }

} // namespace pulsegrid::infrastructure::db
