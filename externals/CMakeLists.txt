find_package(Boost REQUIRED)

add_subdirectory(googletest)
add_subdirectory(fmt)
add_subdirectory(cpr)
add_subdirectory(json)
set(LOGURU_WITH_STREAMS 1)
add_subdirectory(loguru)
