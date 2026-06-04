#include "config_manager.h"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QString>
#include <fstream>
#include <sstream>

namespace BrainLLM {

ConfigManager::ConfigManager()
    : brain_config_(default_brain_config()),
      ui_settings_(default_ui_settings()),
      api_settings_(default_api_settings()),
      log_level_("INFO") {
    load_config("config.ini");
}

bool ConfigManager::load_config(const std::string& filepath) {
    const QString path = QString::fromStdString(filepath);
    if (QFileInfo(path).suffix().compare("ini", Qt::CaseInsensitive) == 0) {
        QSettings settings(path, QSettings::IniFormat);

        settings.beginGroup("brain");
        brain_config_.num_layers = settings.value("num_layers", brain_config_.num_layers).toInt();
        brain_config_.neurons_per_layer = settings.value("neurons_per_layer", brain_config_.neurons_per_layer).toInt();
        brain_config_.learning_rate = settings.value("learning_rate", brain_config_.learning_rate).toFloat();
        brain_config_.max_memory_size = settings.value("max_memory_size", brain_config_.max_memory_size).toInt();
        brain_config_.memory_decay_rate = settings.value("memory_decay_rate", brain_config_.memory_decay_rate).toFloat();
        brain_config_.num_attention_heads = settings.value("num_attention_heads", brain_config_.num_attention_heads).toInt();
        brain_config_.attention_dim = settings.value("attention_dim", brain_config_.attention_dim).toInt();
        brain_config_.vocab_size = settings.value("vocab_size", brain_config_.vocab_size).toInt();
        brain_config_.embedding_dim = settings.value("embedding_dim", brain_config_.embedding_dim).toInt();
        brain_config_.context_length = settings.value("context_length", brain_config_.context_length).toInt();
        brain_config_.batch_size = settings.value("batch_size", brain_config_.batch_size).toInt();
        brain_config_.temperature = settings.value("temperature", brain_config_.temperature).toFloat();
        settings.endGroup();

        settings.beginGroup("ui");
        ui_settings_.dark_mode = settings.value("dark_mode", ui_settings_.dark_mode).toBool();
        ui_settings_.window_width = settings.value("window_width", ui_settings_.window_width).toInt();
        ui_settings_.window_height = settings.value("window_height", ui_settings_.window_height).toInt();
        ui_settings_.maximize_on_start = settings.value("maximize_on_start", ui_settings_.maximize_on_start).toBool();
        ui_settings_.font_size = settings.value("font_size", ui_settings_.font_size).toFloat();
        ui_settings_.theme = settings.value("theme", QString::fromStdString(ui_settings_.theme)).toString().toStdString();
        settings.endGroup();

        settings.beginGroup("api");
        api_settings_.host = settings.value("host", QString::fromStdString(api_settings_.host)).toString().toStdString();
        api_settings_.port = settings.value("port", api_settings_.port).toInt();
        api_settings_.enable_cors = settings.value("enable_cors", api_settings_.enable_cors).toBool();
        api_settings_.max_connections = settings.value("max_connections", api_settings_.max_connections).toInt();
        api_settings_.api_key = settings.value("api_key", QString::fromStdString(api_settings_.api_key)).toString().toStdString();
        settings.endGroup();

        settings.beginGroup("logging");
        log_level_ = settings.value("log_level", QString::fromStdString(log_level_)).toString().toStdString();
        settings.endGroup();

        return true;
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_json(buffer.str());
}

bool ConfigManager::save_config(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    file << to_json();
    return true;
}

BrainConfig ConfigManager::get_brain_config() const {
    return brain_config_;
}

void ConfigManager::set_brain_config(const BrainConfig& config) {
    brain_config_ = config;
}

ConfigManager::UISettings ConfigManager::get_ui_settings() const {
    return ui_settings_;
}

void ConfigManager::set_ui_settings(const UISettings& settings) {
    ui_settings_ = settings;
}

ConfigManager::APISettings ConfigManager::get_api_settings() const {
    return api_settings_;
}

void ConfigManager::set_api_settings(const APISettings& settings) {
    api_settings_ = settings;
}

void ConfigManager::set_log_level(const std::string& level) {
    log_level_ = level;
}

std::string ConfigManager::get_log_level() const {
    return log_level_;
}

BrainConfig ConfigManager::default_brain_config() {
    return {
        8,           // num_layers
        256,         // neurons_per_layer
        0.001f,      // learning_rate
        10000,       // max_memory_size
        0.95f,       // memory_decay_rate
        8,           // num_attention_heads
        512,         // attention_dim
        2048,        // vocab_size
        768,         // embedding_dim
        1024,        // context_length
        32,          // batch_size
        0.7f         // temperature
    };
}

ConfigManager::UISettings ConfigManager::default_ui_settings() {
    return {
        true,        // dark_mode
        1920,        // window_width
        1080,        // window_height
        false,       // maximize_on_start
        12.0f,       // font_size
        "dark"       // theme
    };
}

ConfigManager::APISettings ConfigManager::default_api_settings() {
    return {
        "localhost", // host
        8080,        // port
        true,        // enable_cors
        100,         // max_connections
        ""           // api_key
    };
}

bool ConfigManager::parse_json(const std::string& json_content) {
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json_content), &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonObject brain = root.value("brain").toObject();
    brain_config_.num_layers = brain.value("num_layers").toInt(brain_config_.num_layers);
    brain_config_.neurons_per_layer = brain.value("neurons_per_layer").toInt(brain_config_.neurons_per_layer);
    brain_config_.learning_rate = static_cast<float>(brain.value("learning_rate").toDouble(brain_config_.learning_rate));
    brain_config_.max_memory_size = brain.value("max_memory_size").toInt(brain_config_.max_memory_size);
    brain_config_.memory_decay_rate = static_cast<float>(brain.value("memory_decay_rate").toDouble(brain_config_.memory_decay_rate));
    brain_config_.num_attention_heads = brain.value("attention_heads").toInt(
        brain.value("num_attention_heads").toInt(brain_config_.num_attention_heads));
    brain_config_.attention_dim = brain.value("attention_dim").toInt(brain_config_.attention_dim);
    brain_config_.vocab_size = brain.value("vocab_size").toInt(brain_config_.vocab_size);
    brain_config_.embedding_dim = brain.value("embedding_dim").toInt(brain_config_.embedding_dim);
    brain_config_.context_length = brain.value("context_length").toInt(brain_config_.context_length);
    brain_config_.batch_size = brain.value("batch_size").toInt(brain_config_.batch_size);
    brain_config_.temperature = static_cast<float>(brain.value("temperature").toDouble(brain_config_.temperature));

    const QJsonObject ui = root.value("ui").toObject();
    ui_settings_.dark_mode = ui.value("dark_mode").toBool(ui_settings_.dark_mode);
    ui_settings_.window_width = ui.value("window_width").toInt(ui_settings_.window_width);
    ui_settings_.window_height = ui.value("window_height").toInt(ui_settings_.window_height);
    ui_settings_.maximize_on_start = ui.value("maximize_on_start").toBool(ui_settings_.maximize_on_start);
    ui_settings_.font_size = static_cast<float>(ui.value("font_size").toDouble(ui_settings_.font_size));
    if (ui.contains("theme")) {
        ui_settings_.theme = ui.value("theme").toString(QString::fromStdString(ui_settings_.theme)).toStdString();
    }

    const QJsonObject api = root.value("api").toObject();
    if (api.contains("host")) {
        api_settings_.host = api.value("host").toString(QString::fromStdString(api_settings_.host)).toStdString();
    }
    api_settings_.port = api.value("port").toInt(api_settings_.port);
    api_settings_.enable_cors = api.value("enable_cors").toBool(api_settings_.enable_cors);
    api_settings_.max_connections = api.value("max_connections").toInt(api_settings_.max_connections);
    if (api.contains("api_key")) {
        api_settings_.api_key = api.value("api_key").toString(QString::fromStdString(api_settings_.api_key)).toStdString();
    }

    if (root.contains("logging")) {
        log_level_ = root.value("logging").toObject().value("log_level").toString(QString::fromStdString(log_level_)).toStdString();
    }

    return true;
}

std::string ConfigManager::to_json() const {
    QJsonObject brain;
    brain["num_layers"] = brain_config_.num_layers;
    brain["neurons_per_layer"] = brain_config_.neurons_per_layer;
    brain["learning_rate"] = brain_config_.learning_rate;
    brain["max_memory_size"] = brain_config_.max_memory_size;
    brain["memory_decay_rate"] = brain_config_.memory_decay_rate;
    brain["num_attention_heads"] = brain_config_.num_attention_heads;
    brain["attention_dim"] = brain_config_.attention_dim;
    brain["vocab_size"] = brain_config_.vocab_size;
    brain["embedding_dim"] = brain_config_.embedding_dim;
    brain["context_length"] = brain_config_.context_length;
    brain["batch_size"] = brain_config_.batch_size;
    brain["temperature"] = brain_config_.temperature;

    QJsonObject ui;
    ui["dark_mode"] = ui_settings_.dark_mode;
    ui["window_width"] = ui_settings_.window_width;
    ui["window_height"] = ui_settings_.window_height;
    ui["maximize_on_start"] = ui_settings_.maximize_on_start;
    ui["font_size"] = ui_settings_.font_size;
    ui["theme"] = QString::fromStdString(ui_settings_.theme);

    QJsonObject api;
    api["host"] = QString::fromStdString(api_settings_.host);
    api["port"] = api_settings_.port;
    api["enable_cors"] = api_settings_.enable_cors;
    api["max_connections"] = api_settings_.max_connections;
    api["api_key"] = QString::fromStdString(api_settings_.api_key);

    QJsonObject logging;
    logging["log_level"] = QString::fromStdString(log_level_);

    QJsonObject root;
    root["brain"] = brain;
    root["ui"] = ui;
    root["api"] = api;
    root["logging"] = logging;

    return QJsonDocument(root).toJson(QJsonDocument::Indented).toStdString();
}

} // namespace BrainLLM
