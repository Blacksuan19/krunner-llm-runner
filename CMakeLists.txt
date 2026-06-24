cmake_minimum_required(VERSION 3.28)
project(krunner-llm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Find KDE and Qt packages
find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)

find_package(Qt6 6.6.0 REQUIRED COMPONENTS
    Core
    Network
    Test
    Widgets
    Gui
)

find_package(KF6 6.0.0 REQUIRED COMPONENTS
    CoreAddons
    Runner
    I18n
    Config
    ConfigWidgets
    KCMUtils
)

# Add compile options
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror=return-type
)

# Source directories
add_subdirectory(src)

# # Tests
# set(BUILD_TESTING OFF CACHE BOOL "Enable tests")
# enable_testing()
# add_subdirectory(tests)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
