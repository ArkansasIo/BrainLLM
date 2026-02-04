#include "request_handler.h"
#include <sstream>

namespace BrainLLM {

RequestHandler::RequestHandler(std::shared_ptr<LLMEngine> engine)
    : engine_(engine) {}

QString RequestHandler::handle_request(const QString& method, const QString& path, const QString& body) {
    if (path == "/api/process" && method == "POST") {
        return handle_process(body);
    }
    else if (path == "/api/generate" && method == "POST") {
        return handle_generate(body);
    }
    else if (path == "/api/status" && method == "GET") {
        return handle_status(body);
    }
    else if (path == "/api/memory" && method == "GET") {
        return handle_memory(body);
    }
    else if (path == "/api/config" && method == "GET") {
        return handle_config(body);
    }
    else if (path == "/api/train" && method == "POST") {
        return handle_train(body);
    }
    
    return create_error_response("Endpoint not found");
}

QString RequestHandler::handle_process(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::string input = body.toStdString();
    std::string output = engine_->process_input(input);
    
    return create_json_response(output);
}

QString RequestHandler::handle_generate(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::string prompt = body.toStdString();
    std::string response = engine_->generate_response(prompt);
    
    return create_json_response(response);
}

QString RequestHandler::handle_status(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    auto metrics = engine_->get_metrics();
    
    std::ostringstream oss;
    oss << "{\"status\":\"running\",\"confidence\":" << engine_->get_confidence() 
        << ",\"accuracy\":" << metrics.accuracy << "}";
    
    return QString::fromStdString(oss.str());
}

QString RequestHandler::handle_memory(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::string query = body.toStdString();
    auto memories = engine_->recall_memories(query);
    
    std::ostringstream oss;
    oss << "{\"memories\":[";
    for (size_t i = 0; i < memories.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"content\":\"" << memories[i].content << "\",\"importance\":" 
            << memories[i].importance << "}";
    }
    oss << "]}";
    
    return QString::fromStdString(oss.str());
}

QString RequestHandler::handle_config(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    auto config = engine_->get_config();
    
    std::ostringstream oss;
    oss << "{\"num_layers\":" << config.num_layers 
        << ",\"neurons_per_layer\":" << config.neurons_per_layer 
        << ",\"learning_rate\":" << config.learning_rate << "}";
    
    return QString::fromStdString(oss.str());
}

QString RequestHandler::handle_train(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::vector<std::string> training_data;
    training_data.push_back(body.toStdString());
    
    engine_->train(training_data);
    
    return create_json_response("Training completed");
}

QString RequestHandler::create_json_response(const std::string& message) {
    std::ostringstream oss;
    oss << "{\"message\":\"" << message << "\"}";
    return QString::fromStdString(oss.str());
}

QString RequestHandler::create_error_response(const std::string& error) {
    std::ostringstream oss;
    oss << "{\"error\":\"" << error << "\"}";
    return QString::fromStdString(oss.str());
}

} // namespace BrainLLM
