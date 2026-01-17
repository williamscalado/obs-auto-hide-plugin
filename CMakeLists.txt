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
    set(LIBOBS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/obs-studio/libobs")
    set(SIMDE_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/simde")
    set(OBS_FRONTEND_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/deps/obs-studio/frontend/api")
    
    include_directories(${LIBOBS_INCLUDE_DIR} ${SIMDE_INCLUDE_DIR} ${OBS_FRONTEND_INCLUDE_DIR})
    
    # Link directly to the installed OBS framework
    set(LIBOBS_LIB "/Applications/OBS.app/Contents/Frameworks/libobs.framework")
    set(OBS_FRONTEND_LIB "/Applications/OBS.app/Contents/Frameworks/obs-frontend-api.dylib")
    link_libraries("-F/Applications/OBS.app/Contents/Frameworks" "-framework libobs" ${OBS_FRONTEND_LIB})
else()
    find_package(LibObs REQUIRED)
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
    target_link_libraries(${PROJECT_NAME} PRIVATE LibObs::LibObs)
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
