cmake_minimum_required(VERSION 3.10)

project(SFMLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------
# Client
# --------------------
file(GLOB CLIENT_SRC
    src/*.cpp
)

list(FILTER CLIENT_SRC EXCLUDE REGEX "[Ss]erver\\.cpp$")

add_executable(app ${CLIENT_SRC})

target_include_directories(app PRIVATE
    include
    C:/SFML/include
)

target_link_directories(app PRIVATE
    C:/SFML/lib
)

target_link_libraries(app
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-network
)


# --------------------
# Server
# --------------------
add_executable(server
    src/Server.cpp
    src/Player.cpp
    src/Game.cpp
)

target_include_directories(server PRIVATE
    include
    C:/SFML/include
)

target_link_directories(server PRIVATE
    C:/SFML/lib
)

target_link_libraries(server
    sfml-network
    sfml-system
)


# Put executables in project root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})