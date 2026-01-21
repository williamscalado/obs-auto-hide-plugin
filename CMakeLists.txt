cmake_minimum_required(VERSION 3.20)
project(auto-hide-scenes VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Disable Qt version tagging to allow running with older Qt versions (like OBS internal Qt)
add_compile_definitions(QT_NO_VERSION_TAGGING)

# Manual LibObs configuration for macOS (using local headers and installed App)
if(APPLE)
    # Tenta encontrar via Homebrew primeiro se não existir em deps/
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/deps/obs-studio/libobs")
        set(HOMEBREW_PREFIX "/opt/homebrew")
        if(EXISTS "${HOMEBREW_PREFIX}/include/obs")
            set(LIBOBS_INCLUDE_DIR "${HOMEBREW_PREFIX}/include/obs")
            # Frontend API geralmente fica junto ou em subpasta
            # Tenta encontrar obs-frontend-api.h
            file(GLOB_RECURSE FRONTEND_HEADER "${HOMEBREW_PREFIX}/include/obs/obs-frontend-api.h")
            if(FRONTEND_HEADER)
                get_filename_component(OBS_FRONTEND_INCLUDE_DIR ${FRONTEND_HEADER} DIRECTORY)
            else()
                 # Fallback
                 set(OBS_FRONTEND_INCLUDE_DIR "${HOMEBREW_PREFIX}/include/obs")
            endif()

            message(STATUS "Using Homebrew OBS headers at: ${LIBOBS_INCLUDE_DIR}")
        endif()
    else()
        set(LIBOBS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/obs-studio/libobs")
        set(OBS_FRONTEND_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/obs-studio/UI/obs-frontend-api")
    endif()

    set(SIMDE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/simde")

    # Debug print
    message(STATUS "DEBUG: LIBOBS_INCLUDE_DIR=${LIBOBS_INCLUDE_DIR}")
    message(STATUS "DEBUG: OBS_FRONTEND_INCLUDE_DIR=${OBS_FRONTEND_INCLUDE_DIR}")

    if(NOT EXISTS "${OBS_FRONTEND_INCLUDE_DIR}/obs-frontend-api.h")
         message(WARNING "obs-frontend-api.h NOT FOUND at ${OBS_FRONTEND_INCLUDE_DIR}")
         # Tenta listar diretorio para debug
         execute_process(COMMAND ls -R "${CMAKE_SOURCE_DIR}/deps" OUTPUT_VARIABLE DEPS_LIST)
         message(STATUS "Deps content: ${DEPS_LIST}")
    endif()

    include_directories(${LIBOBS_INCLUDE_DIR} ${SIMDE_INCLUDE_DIR} ${OBS_FRONTEND_INCLUDE_DIR})

    # Link directly to the installed OBS framework or dylib
    # Verifica se existe no path padrao
    if(EXISTS "/Applications/OBS.app/Contents/Frameworks/libobs.framework")
        set(LIBOBS_LIB "/Applications/OBS.app/Contents/Frameworks/libobs.framework")
        set(OBS_FRONTEND_LIB "/Applications/OBS.app/Contents/Frameworks/obs-frontend-api.dylib")
        link_libraries("-F/Applications/OBS.app/Contents/Frameworks" "-framework libobs" ${OBS_FRONTEND_LIB})
    else()
        # Tenta Homebrew lib
        set(LIBOBS_LIB "/opt/homebrew/lib/libobs.dylib")
        set(OBS_FRONTEND_LIB "/opt/homebrew/lib/libobs-frontend-api.dylib") # Nome pode variar
        if(EXISTS ${LIBOBS_LIB})
             message(STATUS "Using Homebrew libs")
             target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBOBS_LIB} ${OBS_FRONTEND_LIB})
        else()
             message(WARNING "Could not find OBS libs on macOS. Build may fail.")
        endif()
    endif()
else()
    # Windows/Linux: Try to find LibObs via standard package
    find_package(LibObs QUIET)

    # If standard package not found, use manual fallback (common for CI/Dev)
    if(NOT LibObs_FOUND)
        message(STATUS "LibObs package not found. Attempting manual configuration using 'deps/' and provided vars...")

        # 1. Headers from local deps or provided variables
        if(DEFINED LIBOBS_INCLUDE_DIR AND DEFINED OBS_FRONTEND_INCLUDE_DIR AND DEFINED SIMDE_INCLUDE_DIR)
            # Use provided paths (from cmake -D flags)
            message(STATUS "Using provided include directories:")
            message(STATUS "  LIBOBS_INCLUDE_DIR: ${LIBOBS_INCLUDE_DIR}")
            message(STATUS "  OBS_FRONTEND_INCLUDE_DIR: ${OBS_FRONTEND_INCLUDE_DIR}")
            message(STATUS "  SIMDE_INCLUDE_DIR: ${SIMDE_INCLUDE_DIR}")
        else()
            # Fallback to deps/ subdirectory
            set(LIBOBS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/obs-studio/libobs")
            set(OBS_FRONTEND_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/obs-studio/UI/obs-frontend-api")
            set(SIMDE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/simde")
            message(STATUS "Using fallback deps/ directories")
        endif()

        if(EXISTS "${LIBOBS_INCLUDE_DIR}" AND EXISTS "${SIMDE_INCLUDE_DIR}")
            include_directories(${LIBOBS_INCLUDE_DIR} ${SIMDE_INCLUDE_DIR} ${OBS_FRONTEND_INCLUDE_DIR})
            message(STATUS "Successfully configured include directories")

            # Debug: Verify frontend header exists
            if(NOT EXISTS "${OBS_FRONTEND_INCLUDE_DIR}/obs-frontend-api.h")
                message(WARNING "obs-frontend-api.h NOT FOUND at ${OBS_FRONTEND_INCLUDE_DIR}")
            else()
                message(STATUS "✓ Found obs-frontend-api.h at ${OBS_FRONTEND_INCLUDE_DIR}")
            endif()
        else()
            message(FATAL_ERROR "LibObs headers not found. Checked: ${LIBOBS_INCLUDE_DIR}")
        endif()
    endif()
endif()

# Plugin sources
set(PLUGIN_SOURCES
    src/plugin-main.cpp
    src/plugin-config.cpp
    src/holyrics-client.cpp
    src/scene-controller.cpp
    src/auto-hide-dock.cpp
    src/settings-dialog.cpp
)

# Create the library
add_library(${PROJECT_NAME} MODULE ${PLUGIN_SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

if(NOT APPLE)
    if(TARGET LibObs::LibObs)
        target_link_libraries(${PROJECT_NAME} PRIVATE LibObs::LibObs)
    else()
        # Fallback: Link against manually provided libraries (set via -DLIBOBS_LIB=...)
        if(LIBOBS_LIB AND OBS_FRONTEND_LIB)
            message(STATUS "Linking against provided libraries:")
            message(STATUS "  LIBOBS_LIB: ${LIBOBS_LIB}")
            message(STATUS "  OBS_FRONTEND_LIB: ${OBS_FRONTEND_LIB}")
            target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBOBS_LIB} ${OBS_FRONTEND_LIB})
        else()
            message(FATAL_ERROR "LibObs libraries not defined. Please set -DLIBOBS_LIB and -DOBS_FRONTEND_LIB.")
        endif()
    endif()
endif()

# Install location
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/obs-plugins/64bit
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/obs-plugins/64bit
)

# Install data (Disabled - No translations used)
# install(DIRECTORY data/locale
#     DESTINATION ${CMAKE_INSTALL_PREFIX}/data/obs-plugins/${PROJECT_NAME}
# )
