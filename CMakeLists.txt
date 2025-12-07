cmake_minimum_required(VERSION 3.10)

# Check for VitaSDK
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

# Project definition
project(f3vita C)

# Include VitaSDK macros
include("${VITASDK}/share/vita.cmake" REQUIRED)

# Build settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# App metadata
set(VITA_APP_NAME "f3vita")
set(VITA_TITLEID  "FVTA00001")
set(VITA_VERSION  "01.00")

# Source files
set(SOURCES
    src/main.c
    src/storage.c
    src/pattern.c
    src/ui.c
    src/debugScreen.c
)

# Include directories
include_directories(include src)

# Create the executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    SceDisplay_stub
    SceCtrl_stub
    SceIofilemgr_stub
    SceAppMgr_stub
    SceSysmodule_stub
    SceRtc_stub
)

# Create SELF (signed executable)
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

# Enable Unsafe Homebrew (Required for filesystem access)
# We apply this to the SELF (Executable) instead of the SFO to avoid the build error.
set(VITA_MAKE_FSELF_FLAGS "-s")

# Create VPK (installable package)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE sce_sys/icon0.png sce_sys/icon0.png
    #FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    #FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    #FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)