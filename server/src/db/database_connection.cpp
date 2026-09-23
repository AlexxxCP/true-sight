#include "db/database_connection.hpp"

pqxx::result DatabaseConnection::exec(const std::string& sql, const pqxx::params& params) {
    pqxx::work tx{connection_};

    pqxx::result res = tx.exec(sql, params);

    tx.commit();

    return res;
};
