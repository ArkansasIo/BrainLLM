#include <iostream>
#include <thread>
#include <chrono>
#include "llm_engine.h"
#include "rest_server.h"
#include "config_manager.h"

int main() {
    std::cout << "=== BrainLLM API Server ===" << std::endl;
    std::cout << "Initializing AI Brain Engine..." << std::endl;
    
    // Load configuration
    BrainLLM::ConfigManager config_manager;
    auto brain_config = config_manager.get_brain_config();
    auto api_config = config_manager.get_api_settings();
    
    // Initialize LLM Engine
    auto llm_engine = std::make_shared<BrainLLM::LLMEngine>(brain_config);
    llm_engine->initialize();
    
    std::cout << "LLM Engine initialized with " << brain_config.num_layers 
              << " layers and " << brain_config.neurons_per_layer << " neurons per layer" << std::endl;
    
    // Start REST API Server
    auto api_server = std::make_shared<BrainLLM::RestServer>(api_config.port);
    api_server->set_llm_engine(llm_engine);
    
    if (api_server->start()) {
        std::cout << "API Server started on " << api_config.host << ":" 
                  << api_config.port << std::endl;
    } else {
        std::cerr << "Failed to start API server" << std::endl;
        return 1;
    }
    
    std::cout << "\nAvailable endpoints:" << std::endl;
    std::cout << "  POST   /api/process    - Process input text" << std::endl;
    std::cout << "  POST   /api/generate   - Generate response from prompt" << std::endl;
    std::cout << "  GET    /api/status     - Get brain status and metrics" << std::endl;
    std::cout << "  GET    /api/memory     - Query memory" << std::endl;
    std::cout << "  GET    /api/config     - Get current configuration" << std::endl;
    std::cout << "  POST   /api/train      - Train the model" << std::endl;
    
    std::cout << "\nServer running... Press Ctrl+C to stop" << std::endl;
    
    // Keep server running
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Periodic memory decay
        if (std::rand() % 100 == 0) {
            // Simulate memory operations
        }
    }
    
    return 0;
}
