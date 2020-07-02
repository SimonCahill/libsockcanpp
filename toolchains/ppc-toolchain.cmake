set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR powerpc)

###
# Set binary paths
###
set(tools /usr/bin)
set(cross powerpc-linux-gnu)

set(CMAKE_C_COMPILER ${tools}/${cross}-gcc)
set(CMAKE_CXX_COMPILER ${tools}/${cross}-g++)
set(CMAKE_LD ${tools}/${cross}-ld)
set(CMAKE_STRIP ${tools}/${cross}-strip)
set(CMAKE_AR ${tools}/${cross}-ar)

add_definitions(
    -DPOWERPC
)

###
# Set source and lib paths
###

#include_directories(SYSTEM )
