#include "request_handler.h"
#include "lua_scripting_system.h"
#include "voice_audio_system.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QRegularExpression>
#include <sstream>

namespace BrainLLM {

namespace {

QString json_escape(const std::string& value) {
    return QString::fromStdString(value).toHtmlEscaped()
        .replace("\\", "\\\\")
        .replace("\"", "\\\"");
}

QString extract_text_field(const QString& body, const QString& field, const QString& fallback = QString()) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains(field)) {
            return obj.value(field).toString();
        }
    }
    return fallback.isNull() ? body : fallback;
}

int extract_int_field(const QString& body, const QString& field, int fallback) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains(field)) {
            return obj.value(field).toInt(fallback);
        }
    }
    return fallback;
}

QString compact_json(const QJsonObject& object) {
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QString compact_json(const QJsonArray& array) {
    return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QString extract_prompt(const QString& body) {
    const QString prompt = extract_text_field(body, "prompt", QString());
    return prompt.isEmpty() ? body : prompt;
}

QString extract_chat_prompt(const QString& body) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return body;
    }

    const QJsonArray messages = doc.object().value("messages").toArray();
    QStringList prompt_parts;
    for (const QJsonValue& value : messages) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject message = value.toObject();
        const QString role = message.value("role").toString("user");
        QString content;

        if (message.value("content").isString()) {
            content = message.value("content").toString();
        } else if (message.value("content").isArray()) {
            QStringList content_parts;
            for (const QJsonValue& part_value : message.value("content").toArray()) {
                if (part_value.isObject()) {
                    const QJsonObject part = part_value.toObject();
                    if (part.value("type").toString() == "text") {
                        content_parts.push_back(part.value("text").toString());
                    }
                }
            }
            content = content_parts.join("\n");
        }

        if (!content.trimmed().isEmpty()) {
            prompt_parts.push_back(role + ": " + content.trimmed());
        }
    }

    return prompt_parts.isEmpty() ? body : prompt_parts.join("\n");
}

QString model_name_from_body(const QString& body) {
    return extract_text_field(body, "model", "brainllm-local");
}

} // namespace

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
    else if (path == "/api/wolfram" && method == "POST") {
        return handle_wolfram(body);
    }
    else if (path == "/api/speech/status" && method == "GET") {
        return handle_speech_status(body);
    }
    else if (path == "/api/speech/speak" && method == "POST") {
        return handle_speech_speak(body);
    }
    else if (path == "/api/speech/synthesize" && method == "POST") {
        return handle_speech_synthesize(body);
    }
    else if (path == "/api/speech/recognize" && method == "POST") {
        return handle_speech_recognize(body);
    }
    else if (path == "/api/scripts" && method == "GET") {
        return handle_scripts(body);
    }
    else if (path == "/api/scripts/execute" && method == "POST") {
        return handle_script_execute(body);
    }
    else if (path == "/v1/models" && method == "GET") {
        return handle_models(body);
    }
    else if (path == "/v1/chat/completions" && method == "POST") {
        return handle_chat_completions(body);
    }
    else if (path == "/v1/completions" && method == "POST") {
        return handle_completions(body);
    }
    
    return create_error_response("Endpoint not found");
}

QString RequestHandler::handle_process(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::string input = extract_text_field(body, "input").toStdString();
    std::string output = engine_->process_input(input);

    return QString("{\"input\":\"%1\",\"output\":\"%2\",\"confidence\":%3}")
        .arg(json_escape(input))
        .arg(json_escape(output))
        .arg(engine_->get_confidence());
}

QString RequestHandler::handle_generate(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::string prompt = extract_text_field(body, "prompt").toStdString();
    int max_tokens = extract_int_field(body, "max_tokens", 100);
    std::string response = engine_->generate_response(prompt, max_tokens);
    
    return QString("{\"prompt\":\"%1\",\"response\":\"%2\",\"confidence\":%3}")
        .arg(json_escape(prompt))
        .arg(json_escape(response))
        .arg(engine_->get_confidence());
}

QString RequestHandler::handle_status(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    auto metrics = engine_->get_metrics();
    
    std::ostringstream oss;
    oss << "{\"status\":\"running\",\"confidence\":" << engine_->get_confidence()
        << ",\"accuracy\":" << metrics.accuracy
        << ",\"cpu_usage\":" << metrics.cpu_usage
        << ",\"memory_usage\":" << metrics.memory_usage
        << ",\"tokens_processed\":" << metrics.tokens_processed << "}";
    
    return QString::fromStdString(oss.str());
}

QString RequestHandler::handle_memory(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::string query = extract_text_field(body, "query", body).toStdString();
    auto memories = engine_->recall_memories(query);
    
    std::ostringstream oss;
    oss << "{\"memories\":[";
    for (size_t i = 0; i < memories.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "{\"content\":\"" << json_escape(memories[i].content).toStdString()
            << "\",\"importance\":" << memories[i].importance
            << ",\"category\":\"" << memories[i].category << "\"}";
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
        << ",\"learning_rate\":" << config.learning_rate
        << ",\"attention_heads\":" << config.num_attention_heads
        << ",\"embedding_dim\":" << config.embedding_dim
        << ",\"context_length\":" << config.context_length << "}";
    
    return QString::fromStdString(oss.str());
}

QString RequestHandler::handle_train(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }
    
    std::vector<std::string> training_data;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && doc.isObject() && doc.object().contains("samples")) {
        for (const auto& item : doc.object().value("samples").toArray()) {
            training_data.push_back(item.toString().toStdString());
        }
    } else {
        training_data.push_back(extract_text_field(body, "text").toStdString());
    }
    
    engine_->train(training_data);
    
    return QString("{\"message\":\"Training completed\",\"samples\":%1,\"confidence\":%2}")
        .arg(static_cast<int>(training_data.size()))
        .arg(engine_->get_confidence());
}

QString RequestHandler::handle_wolfram(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }

    const QString query = extract_text_field(body, "query", body).trimmed();
    const QString mode = extract_text_field(body, "mode", "llm").trimmed().toLower();
    const bool llm_format = mode != "short";
    const QString answer = QString::fromStdString(
        engine_->query_wolfram_alpha(query.toStdString(), llm_format));

    QJsonObject response;
    response["query"] = query;
    response["mode"] = llm_format ? "llm" : "short";
    response["answer"] = answer;
    response["confidence"] = engine_->get_confidence();
    return compact_json(response);
}

QString RequestHandler::handle_speech_status(const QString& body) {
    Q_UNUSED(body);
    VoiceAudioSystem speech;
    const VoiceProfile profile = speech.get_voice_profile();

    QJsonObject response;
    response["available"] = speech.is_platform_speech_available();
    response["backend"] = "Windows System.Speech";
    response["language"] = QString::fromStdString(profile.language);
    response["voice"] = QString::fromStdString(profile.display_name);
    response["supports_text_to_speech"] = speech.is_platform_speech_available();
    response["supports_speech_recognition"] = speech.is_platform_speech_available();
    return compact_json(response);
}

QString RequestHandler::handle_speech_speak(const QString& body) {
    VoiceAudioSystem speech;
    const QString text = extract_text_field(body, "text", body).trimmed();
    const TextToSpeechResult result = speech.speak_text(text.toStdString());

    QJsonObject response;
    response["success"] = result.success;
    response["message"] = QString::fromStdString(result.message);
    response["error"] = QString::fromStdString(result.error);
    return compact_json(response);
}

QString RequestHandler::handle_speech_synthesize(const QString& body) {
    VoiceAudioSystem speech;
    const QString text = extract_text_field(body, "text", body).trimmed();
    QString output_path = extract_text_field(body, "output_path", "assets/audio/generated/speech.wav").trimmed();
    if (output_path.isEmpty()) {
        output_path = "assets/audio/generated/speech.wav";
    }

    const TextToSpeechResult result = speech.synthesize_to_wav(
        text.toStdString(),
        output_path.toStdString());

    QJsonObject response;
    response["success"] = result.success;
    response["output_path"] = QString::fromStdString(result.output_path);
    response["message"] = QString::fromStdString(result.message);
    response["error"] = QString::fromStdString(result.error);
    return compact_json(response);
}

QString RequestHandler::handle_speech_recognize(const QString& body) {
    VoiceAudioSystem speech;
    const int timeout_seconds = extract_int_field(body, "timeout_seconds", 6);
    const SpeechRecognitionResult result = speech.recognize_once(timeout_seconds);

    QJsonObject response;
    response["success"] = result.success;
    response["transcript"] = QString::fromStdString(result.transcript);
    response["confidence"] = result.confidence;
    response["error"] = QString::fromStdString(result.error);
    return compact_json(response);
}

QString RequestHandler::handle_scripts(const QString& body) {
    Q_UNUSED(body);
    LuaScriptingSystem scripts;
    QJsonArray items;

    for (const LuaScriptDefinition& script : scripts.get_scripts()) {
        QJsonObject item;
        item["script_id"] = QString::fromStdString(script.script_id);
        item["name"] = QString::fromStdString(script.name);
        item["path"] = QString::fromStdString(script.script_path);
        item["entry_point"] = QString::fromStdString(script.entry_point);
        item["enabled"] = script.enabled;
        items.append(item);
    }

    QJsonObject response;
    response["scripts"] = items;
    return compact_json(response);
}

QString RequestHandler::handle_script_execute(const QString& body) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return create_error_response("Expected JSON object with script_id and optional args.");
    }

    const QJsonObject obj = doc.object();
    const QString script_id = obj.value("script_id").toString().trimmed();
    if (script_id.isEmpty()) {
        return create_error_response("script_id is required.");
    }

    std::map<std::string, std::string> args;
    const QJsonObject arg_obj = obj.value("args").toObject();
    for (auto it = arg_obj.begin(); it != arg_obj.end(); ++it) {
        args[it.key().toStdString()] = it.value().toVariant().toString().toStdString();
    }

    LuaScriptingSystem scripts;
    const ScriptExecutionResult result = scripts.execute_script(script_id.toStdString(), args);

    QJsonObject response;
    response["success"] = result.success;
    response["script_id"] = QString::fromStdString(result.script_id);
    response["output"] = QString::fromStdString(result.output);
    response["error"] = QString::fromStdString(result.error);
    return compact_json(response);
}

QString RequestHandler::handle_models(const QString& body) {
    Q_UNUSED(body);

    QJsonArray models;
    QJsonObject local_model;
    local_model["id"] = "brainllm-local";
    local_model["object"] = "model";
    local_model["created"] = 0;
    local_model["owned_by"] = "BrainLLM";
    models.append(local_model);

    QJsonObject chat_model;
    chat_model["id"] = "brainllm-chat";
    chat_model["object"] = "model";
    chat_model["created"] = 0;
    chat_model["owned_by"] = "BrainLLM";
    models.append(chat_model);

    QJsonObject response;
    response["object"] = "list";
    response["data"] = models;
    return compact_json(response);
}

QString RequestHandler::handle_chat_completions(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }

    const QString prompt = extract_chat_prompt(body).trimmed();
    const int max_tokens = extract_int_field(body, "max_tokens", 180);
    const QString model = model_name_from_body(body);
    const QString content = QString::fromStdString(
        engine_->generate_response(prompt.toStdString(), max_tokens));

    QJsonObject message;
    message["role"] = "assistant";
    message["content"] = content;

    QJsonObject choice;
    choice["index"] = 0;
    choice["message"] = message;
    choice["finish_reason"] = "stop";

    QJsonObject usage;
    usage["prompt_tokens"] = prompt.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    usage["completion_tokens"] = content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    usage["total_tokens"] = usage["prompt_tokens"].toInt() + usage["completion_tokens"].toInt();

    QJsonObject response;
    response["id"] = "chatcmpl-brainllm-local";
    response["object"] = "chat.completion";
    response["created"] = static_cast<qint64>(QDateTime::currentSecsSinceEpoch());
    response["model"] = model;
    response["choices"] = QJsonArray{choice};
    response["usage"] = usage;
    response["confidence"] = engine_->get_confidence();
    return compact_json(response);
}

QString RequestHandler::handle_completions(const QString& body) {
    if (!engine_) {
        return create_error_response("Engine not initialized");
    }

    const QString prompt = extract_prompt(body).trimmed();
    const int max_tokens = extract_int_field(body, "max_tokens", 180);
    const QString model = model_name_from_body(body);
    const QString text = QString::fromStdString(
        engine_->generate_response(prompt.toStdString(), max_tokens));

    QJsonObject choice;
    choice["index"] = 0;
    choice["text"] = text;
    choice["finish_reason"] = "stop";

    QJsonObject usage;
    usage["prompt_tokens"] = prompt.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    usage["completion_tokens"] = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
    usage["total_tokens"] = usage["prompt_tokens"].toInt() + usage["completion_tokens"].toInt();

    QJsonObject response;
    response["id"] = "cmpl-brainllm-local";
    response["object"] = "text_completion";
    response["created"] = static_cast<qint64>(QDateTime::currentSecsSinceEpoch());
    response["model"] = model;
    response["choices"] = QJsonArray{choice};
    response["usage"] = usage;
    response["confidence"] = engine_->get_confidence();
    return compact_json(response);
}

QString RequestHandler::create_json_response(const std::string& message) {
    return QString("{\"message\":\"%1\"}").arg(json_escape(message));
}

QString RequestHandler::create_error_response(const std::string& error) {
    return QString("{\"error\":\"%1\"}").arg(json_escape(error));
}

} // namespace BrainLLM
