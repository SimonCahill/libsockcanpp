set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

###
# Set binary paths
###
set(tools /usr/bin)
set(cross x86_64-linux-gnu)

set(CMAKE_C_COMPILER ${tools}/${cross}-gcc)
set(CMAKE_CXX_COMPILER ${tools}/${cross}-g++)
set(CMAKE_LD ${tools}/${cross}-ld)
set(CMAKE_STRIP ${tools}/${cross}-strip)
set(CMAKE_AR ${tools}/${cross}-ar)

###
# Set source and lib paths
###

#include_directories(SYSTEM )
