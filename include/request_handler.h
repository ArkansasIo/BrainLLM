#pragma once

#include "rest_server.h"
#include <QString>

namespace BrainLLM {

class RequestHandler {
public:
    RequestHandler(std::shared_ptr<LLMEngine> engine);
    ~RequestHandler() = default;
    
    QString handle_request(const QString& method, const QString& path, const QString& body);
    
private:
    std::shared_ptr<LLMEngine> engine_;
    
    // Endpoint handlers
    QString handle_process(const QString& body);
    QString handle_generate(const QString& body);
    QString handle_status(const QString& body);
    QString handle_memory(const QString& body);
    QString handle_config(const QString& body);
    QString handle_train(const QString& body);
    
    // Helper methods
    QString create_json_response(const std::string& message);
    QString create_error_response(const std::string& error);
};

} // namespace BrainLLM
