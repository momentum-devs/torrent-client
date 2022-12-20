set(SOURCES
        src/HttpClientImpl.cpp
        src/HttpClientFactory.cpp
        )

add_library(httpClient ${SOURCES})

target_include_directories(httpClient
        PUBLIC
        "include"
        PRIVATE
        "include/httpClient"
        ${CPR_INCLUDE_DIR}
        )

target_link_libraries(httpClient
        PUBLIC
        cpr::cpr
        collection
        )

target_compile_options(httpClient PUBLIC ${FLAGS})
