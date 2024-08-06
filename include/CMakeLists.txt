cmake_minimum_required(VERSION 3.14)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.23")
    target_sources(${PROJECT_NAME}
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
        FILES
            CanDriver.hpp
            CanId.hpp
            CanMessage.hpp
    )

    if (TARGET sockcanpp_test)
        target_sources(sockcanpp_test
            PUBLIC FILE_SET HEADERS
            BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
            FILES
                CanDriver.hpp
                CanId.hpp
                CanMessage.hpp
        )
    endif()
else()
    include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
    )

    if (TARGET sockcanpp_test)
        target_include_directories(sockcanpp_test
            PUBLIC
            ${CMAKE_CURRENT_LIST_DIR}
        )
    endif()
endif()
