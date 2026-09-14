cmake_minimum_required(VERSION 3.20)

project(TrueSightClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(QCORO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(QCORO_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(USE_QT_VERSION 6 CACHE STRING "" FORCE)

find_package(Qt6 REQUIRED COMPONENTS Quick Network WebSockets)
find_package(OpenSSL REQUIRED)
add_subdirectory(external/qcoro)

qt_standard_project_setup(REQUIRES 6.8)
qcoro_enable_coroutines()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE QML_FILES
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.qml"
)

qt_add_executable(TrueSightClient
    ${SOURCES}
)

qt_add_qml_module(TrueSightClient
    URI TrueSight
    QML_FILES
        ${QML_FILES}
)

target_include_directories(TrueSightClient PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(TrueSightClient PRIVATE
    Qt6::Quick
    Qt6::Network
    Qt6::WebSockets
    OpenSSL::SSL
    OpenSSL::Crypto
    QCoro6::Core
    QCoro6::Network
    QCoro6::Coro
)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASan and TSan cannot be enabled together")
endif()

if(ENABLE_ASAN)
    target_compile_options(TrueSightClient PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
    )

    target_link_options(TrueSightClient PRIVATE
        -fsanitize=address
    )
endif()


if(ENABLE_TSAN)
    target_compile_options(TrueSightClient PRIVATE
        -fsanitize=thread
        -fno-omit-frame-pointer
    )

    target_link_options(TrueSightClient PRIVATE
        -fsanitize=thread
    )
endif()

if(ENABLE_UBSAN)
    target_compile_options(TrueSightClient PRIVATE
        -fsanitize=undefined
        -fno-omit-frame-pointer
    )

    target_link_options(TrueSightClient PRIVATE
        -fsanitize=undefined
    )
endif()



