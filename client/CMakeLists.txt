# ---- Networking Layer (internal static library) ----
add_library(client_net STATIC)

target_sources(client_net PRIVATE
    net/socket.hpp
)

if (WIN32)
    target_sources(client_net PRIVATE net/socket_windows.cpp)
else()
    target_sources(client_net PRIVATE net/socket_linux.cpp)
endif()

target_link_libraries(client_net
    PRIVATE
        project_options
        project_warnings
)

target_include_directories(client_net
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---- Client Executable ----
add_executable(client
    main.cpp
)

find_package(ftxui CONFIG REQUIRED)

target_link_libraries(client
    PRIVATE
        project_options
        project_warnings
        client_net
        ftxui::screen
        ftxui::dom
        ftxui::component
)
