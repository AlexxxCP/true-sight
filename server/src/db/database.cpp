#include "db/database.hpp"
#include "db/database_connection.hpp"

pqxx::result Database::exec(std::string query, pqxx::params params) {
    DatabaseConnection db_conn{connection_string_};
    pqxx::result result = db_conn.exec(query, params);

    return result;
};
