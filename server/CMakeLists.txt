cmake_minimum_required(VERSION 3.20)

project(TrueSightServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED COMPONENTS system json)
find_package(libpqxx REQUIRED)
find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(TrueSightServer
    ${SOURCES}
)

target_include_directories(TrueSightServer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(TrueSightServer PRIVATE
    Boost::system
    Boost::json
    libpqxx::pqxx
    OpenSSL::SSL
    OpenSSL::Crypto
)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASan and TSan cannot be enabled together")
endif()

if(ENABLE_ASAN)
    target_compile_options(TrueSightServer PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
    )

    target_link_options(TrueSightServer PRIVATE
        -fsanitize=address
    )
endif()

if(ENABLE_TSAN)
    target_compile_options(TrueSightServer PRIVATE
        -fsanitize=thread
        -fno-omit-frame-pointer
    )

    target_link_options(TrueSightServer PRIVATE
        -fsanitize=thread
    )
endif()

if(ENABLE_UBSAN)
    target_compile_options(TrueSightServer PRIVATE
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )

    target_link_options(TrueSightServer PRIVATE
        -fsanitize=undefined
    )
endif()


