#include <iostream>
#include <QCoreApplication>
#include "llm_engine.h"
#include "rest_server.h"
#include "config_manager.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

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
    std::cout << "  GET    /api/endpoints  - List available API endpoints" << std::endl;
    std::cout << "  GET    /api/algorithms - List active local LLM algorithms" << std::endl;
    std::cout << "  POST   /api/train      - Train the model" << std::endl;
    std::cout << "  POST   /api/chat       - Chat endpoint for web/client use" << std::endl;
    std::cout << "  POST   /api/assistant  - Wake-word assistant endpoint" << std::endl;
    std::cout << "  POST   /api/reset      - Reset brain state and memory" << std::endl;
    std::cout << "  GET/POST /api/airllm/config - Read or update AirLLM config" << std::endl;
    std::cout << "  POST   /api/wolfram    - Query Wolfram Alpha external knowledge" << std::endl;
    std::cout << "  GET    /api/speech/status - Get speech backend status" << std::endl;
    std::cout << "  POST   /api/speech/speak - Speak text through OS text-to-speech" << std::endl;
    std::cout << "  POST   /api/speech/synthesize - Synthesize text to WAV" << std::endl;
    std::cout << "  POST   /api/speech/recognize - Recognize speech from microphone" << std::endl;
    std::cout << "  GET    /api/scripts    - List registered Lua script plans" << std::endl;
    std::cout << "  POST   /api/scripts/execute - Execute a registered Lua script plan" << std::endl;
    std::cout << "  GET    /v1/models      - OpenAI-style model list" << std::endl;
    std::cout << "  POST   /v1/completions - OpenAI-style text completion" << std::endl;
    std::cout << "  POST   /v1/chat/completions - OpenAI-style chat completion" << std::endl;
    std::cout << "  GET    /client         - Browser client UI" << std::endl;
    
    std::cout << "\nServer running... Press Ctrl+C to stop" << std::endl;

    return app.exec();
}
