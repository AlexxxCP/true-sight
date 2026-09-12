#pragma once

#include "shared.hpp"
#include "db/database_connection.hpp"
#include <exception>
#include <pqxx/pqxx>

class Database {
public:
    explicit Database(
        std::string connection_string,
        std::size_t workers
    )
        : connection_string_{std::move(connection_string)},
          pool_{workers}
    {
        if (workers == 0) {
            throw std::runtime_error("A database with 0 workers is not supported!");
        }
    }

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    Database(Database&&) = delete;
    Database& operator=(Database&&) = delete;

    pqxx::result exec(
        std::string query,
        pqxx::params params
    );

    template<typename CompletionToken>
    auto exec_async(
        std::string query,
        pqxx::params params,
        CompletionToken&& token
    );

private:
    std::string connection_string_;
    asio::thread_pool pool_;
};

template<typename CompletionToken>
auto Database::exec_async(std::string query, pqxx::params params, CompletionToken&& token) {
    return asio::async_initiate<
        CompletionToken,
        void(std::exception_ptr, pqxx::result)
    >(
        [this,
        query = std::move(query),
        params = std::move(params)] (auto handler) mutable {
            auto executor = asio::get_associated_executor(handler);
            auto work_guard = asio::make_work_guard(handler);

            asio::post(
                pool_,
                [this,
                query = std::move(query),
                params = std::move(params),
                handler = std::move(handler),
                executor = std::move(executor),
                work_guard = std::move(work_guard)] () mutable {
                    try {
                        thread_local DatabaseConnection db_conn{connection_string_};
                        pqxx::result res = db_conn.exec(query, params);

                        asio::post(
                            executor,
                            [result = std::move(res),
                             work_guard = std::move(work_guard),
                             handler = std::move(handler)]() mutable {
                                std::move(handler)(std::exception_ptr{}, std::move(result));
                            }
                        );
                    } catch (...) {
                        auto ep = std::current_exception();

                        asio::post(
                            executor,
                            [handler = std::move(handler),
                             work_guard = std::move(work_guard),
                             ep = std::move(ep)]() mutable {
                                std::move(handler)(ep, pqxx::result{});
                            }
                        );
                    }
                }
            );
        },
        token
    );
};
