cmake_minimum_required(VERSION 3.23)

target_sources(${PROJECT_NAME}
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
    FILES 
        CanDriver.hpp
        CanId.hpp
        CanMessage.hpp
        exceptions/CanCloseException.hpp
        exceptions/CanException.hpp
        exceptions/CanInitException.hpp
        exceptions/InvalidSocketException.hpp
)

if (TARGET sockcanpp_test)
    target_sources(sockcanpp_test
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}
        FILES 
            CanDriver.hpp
            CanId.hpp
            CanMessage.hpp
            exceptions/CanCloseException.hpp
            exceptions/CanException.hpp
            exceptions/CanInitException.hpp
            exceptions/InvalidSocketException.hpp
    )
endif()
