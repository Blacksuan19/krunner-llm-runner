# LLM Client Library
add_library(llmclient STATIC)
target_sources(llmclient
    PRIVATE
    llmclient.cpp
    llmclient.hpp
)
target_include_directories(llmclient
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(llmclient
    PUBLIC
    Qt6::Core
    Qt6::Network
)
set_target_properties(llmclient PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Runner Plugin
kcoreaddons_add_plugin(krunner_llm
    SOURCES
    llmrunner.cpp
    llmrunner.hpp
    plasma-runner-llm.json
    INSTALL_NAMESPACE "kf6/krunner"
)

target_link_libraries(krunner_llm
    PRIVATE
    KF6::Runner
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigWidgets
    Qt6::Core
    Qt6::Network
    Qt6::Widgets
    Qt6::Gui
    llmclient
)

# Configuration UI
kcoreaddons_add_plugin(kcm_krunner_llm
    SOURCES
    llmconfig.cpp
    llmconfig.hpp
    kcm_krunner_llm.json
    INSTALL_NAMESPACE "kf6/krunner/kcms"
)
set_target_properties(kcm_krunner_llm PROPERTIES AUTOUIC ON)

ki18n_wrap_ui(kcm_krunner_llm llmconfig.ui)

target_link_libraries(kcm_krunner_llm
    PRIVATE
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::KCMUtils
    KF6::I18n
    Qt6::Network
    Qt6::Widgets
)
