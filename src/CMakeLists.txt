target_sources(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/CanDriver.cpp
)

if (TARGET sockcanpp_test)
    target_sources(sockcanpp_test
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/CanDriver.cpp
    )
endif()

add_compile_options(
    -Wno-unknown-pragmas
)
