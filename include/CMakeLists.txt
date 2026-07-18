cmake_minimum_required(VERSION 3.23)

if (sockcanpp_NO_INCL_DIR)
    set(sockcanpp_HEADER_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/sockcanpp)
else()
    set(sockcanpp_HEADER_BASE_DIR ${CMAKE_CURRENT_LIST_DIR})
endif()

set(sockcanpp_HEADERS
    sockcanpp/CanDriver.hpp
    sockcanpp/CanFdMessage.hpp
    sockcanpp/CanId.hpp
    sockcanpp/CanMessage.hpp
    sockcanpp/EnumCheck.hpp
    sockcanpp/TlOptional.hpp
    sockcanpp/exceptions/CanCloseException.hpp
    sockcanpp/exceptions/CanException.hpp
    sockcanpp/exceptions/CanInitException.hpp
    sockcanpp/exceptions/InvalidSocketException.hpp
    sockcanpp/can_errors/CanControllerError.hpp
    sockcanpp/can_errors/CanProtocolError.hpp
    sockcanpp/can_errors/CanTransceiverError.hpp
)

if (sockcanpp_CANXL_SUPPORT)
    list(APPEND sockcanpp_HEADERS sockcanpp/CanXlMessage.hpp)
endif()

target_sources(${PROJECT_NAME}
    PUBLIC FILE_SET HEADERS
    BASE_DIRS ${sockcanpp_HEADER_BASE_DIR}
    FILES ${sockcanpp_HEADERS}
)

if (TARGET sockcanpp_test)
    target_sources(sockcanpp_test
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${sockcanpp_HEADER_BASE_DIR}
        FILES ${sockcanpp_HEADERS}
    )
endif()
