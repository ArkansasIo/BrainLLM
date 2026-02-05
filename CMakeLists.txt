cmake_minimum_required(VERSION 3.16)
project(BrainLLM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 Configuration
find_package(Qt6 COMPONENTS 
    Core 
    Gui 
    Widgets 
    Network 
    Concurrent
    REQUIRED
)

# Third-party libraries
find_package(nlohmann_json 3.2.0 QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, will use header-only")
endif()

# Core LLM Brain Library
add_library(brain_core
    src/brain/neural_network.cpp
    src/brain/memory_system.cpp
    src/brain/attention_mechanism.cpp
    src/brain/llm_engine.cpp
    src/settings/config_manager.cpp
)

target_include_directories(brain_core PUBLIC include)

# Advanced Brain Modules
add_library(advanced_brain
    src/advanced/robotics_laws.cpp
    src/advanced/sentience.cpp
    src/advanced/advanced_learning.cpp
    src/advanced/advanced_architectures.cpp
    src/advanced/knowledge_reasoning.cpp
    src/advanced/dialogue_personality.cpp
    src/advanced/safety_security.cpp
    src/advanced/comprehensive_brain.cpp
)

target_link_libraries(advanced_brain PUBLIC brain_core)
target_include_directories(advanced_brain PUBLIC include)

# Cognitive & Linguistic Modules (v3.0)
add_library(cognitive_linguistic
    src/human_brain/cognitive_processes.cpp
    src/human_brain/english_processor.cpp
    src/human_brain/multilingual_processor.cpp
    src/human_brain/grammar_linguistics.cpp
    src/human_brain/integrated_cognitive_module.cpp
)

target_link_libraries(cognitive_linguistic PUBLIC brain_core advanced_brain)
target_include_directories(cognitive_linguistic PUBLIC include)

# Quantum Computing Module (v4.0)
add_library(quantum_computing
    src/quantum/quantum_computing.cpp
)

target_link_libraries(quantum_computing PUBLIC brain_core)
target_include_directories(quantum_computing PUBLIC include)

# Quantum LLM Integration (v4.0)
add_library(quantum_llm
    src/quantum/quantum_llm_integration.cpp
)

target_link_libraries(quantum_llm PUBLIC brain_core cognitive_linguistic quantum_computing)
target_include_directories(quantum_llm PUBLIC include)

# REST API Server
add_library(api_server
    src/api/rest_server.cpp
    src/api/request_handler.cpp
    src/api/endpoints.cpp
)

target_link_libraries(api_server PUBLIC brain_core advanced_brain cognitive_linguistic Qt6::Network Qt6::Core)
target_include_directories(api_server PUBLIC include)

# GUI Application
add_executable(BrainLLM_GUI
    gui/main_window.cpp
    gui/editor_widget.cpp
    gui/console_widget.cpp
    gui/visualization_widget.cpp
    gui/menu_system.cpp
    gui/settings_dialog.cpp
    src/main_gui.cpp
)

target_link_libraries(BrainLLM_GUI
    brain_core
    advanced_brain
    cognitive_linguistic
    quantum_computing
    quantum_llm
    api_server
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
)

target_include_directories(BrainLLM_GUI PUBLIC include)

# API Server Executable
add_executable(BrainLLM_API
    src/main_api.cpp
)

target_link_libraries(BrainLLM_API
    api_server
    brain_core
    advanced_brain
    cognitive_linguistic
    quantum_computing
    quantum_llm
    Qt6::Core
    Qt6::Network
)

target_include_directories(BrainLLM_API PUBLIC include)
