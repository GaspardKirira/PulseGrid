/**
 *
 *  @file DatabaseFactory.hpp
 *  @author Gaspard Kirira
 *
 *  PulseGrid
 *
 *  Database factory for infrastructure bootstrapping.
 *
 */

#ifndef PULSEGRID_INFRASTRUCTURE_DB_DATABASE_FACTORY_HPP
#define PULSEGRID_INFRASTRUCTURE_DB_DATABASE_FACTORY_HPP

#include <memory>
#include <string>

#include <vix/config/Config.hpp>
#include <vix/db/Database.hpp>

namespace pulsegrid::infrastructure::db
{
  class DatabaseFactory
  {
  public:
    /**
     * @brief Build a database instance from Vix config and apply migrations.
     */
    [[nodiscard]] static std::shared_ptr<vix::db::Database> make(
        const vix::config::Config &config);

    /**
     * @brief Build a SQLite database instance with PulseGrid defaults
     *        and apply migrations.
     */
    [[nodiscard]] static std::shared_ptr<vix::db::Database> make_sqlite_default();

  private:
    static void apply_migrations(
        vix::db::Database &database,
        const std::string &migrations_dir);
  };

} // namespace pulsegrid::infrastructure::db

#endif // PULSEGRID_INFRASTRUCTURE_DB_DATABASE_FACTORY_HPP
