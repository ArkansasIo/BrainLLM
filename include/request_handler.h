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
    QString handle_endpoints(const QString& body);
    QString handle_algorithms(const QString& body);
    QString handle_train(const QString& body);
    QString handle_chat(const QString& body);
    QString handle_reset(const QString& body);
    QString handle_airllm_config(const QString& method, const QString& body);
    QString handle_models(const QString& body);
    QString handle_chat_completions(const QString& body);
    QString handle_completions(const QString& body);
    QString handle_wolfram(const QString& body);
    QString handle_speech_status(const QString& body);
    QString handle_speech_speak(const QString& body);
    QString handle_speech_synthesize(const QString& body);
    QString handle_speech_recognize(const QString& body);
    QString handle_scripts(const QString& body);
    QString handle_script_execute(const QString& body);
    
    // Helper methods
    QString create_json_response(const std::string& message);
    QString create_error_response(const std::string& error);
};

} // namespace BrainLLM
