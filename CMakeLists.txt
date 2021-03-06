cmake_minimum_required(VERSION 2.6)

project (clic_project)
option(USE_CLANG "build application with clang" ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Boost 1.42 COMPONENTS iostreams REQUIRED)
find_package(ZLIB REQUIRED)
find_package(LibClang REQUIRED)
find_package(BerkeleyDB REQUIRED)

include_directories(
    SYSTEM
    ${Boost_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
    ${LibClang_INCLUDE_DIRS}
    ${DB_INCLUDE_DIRS}
)

add_library(common STATIC clic_parser.cpp clic_printer.cpp ClicDb.cpp)

add_executable(clic_clear clic_clear.cpp)
target_link_libraries(clic_clear
    common
    ${DB_LIBRARIES}
)

add_executable(clic_add clic_add.cpp)
target_link_libraries(clic_add
    common
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${DB_LIBRARIES}
    ${LibClang_LIBRARIES}
)

add_executable(clic_rm clic_rm.cpp)
target_link_libraries(clic_rm
    common
    ${Boost_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${DB_LIBRARIES}
)

add_executable(clic_query clic_query.cpp)
target_link_libraries(clic_query
    ${LibClang_LIBRARIES}
)

install(PROGRAMS clic_update.sh clic_update.py
    DESTINATION bin
)
install(TARGETS clic_rm clic_add clic_clear
    RUNTIME DESTINATION bin
)
