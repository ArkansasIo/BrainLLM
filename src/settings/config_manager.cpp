#include "config_manager.h"
#include <fstream>
#include <sstream>

namespace BrainLLM {

ConfigManager::ConfigManager()
    : brain_config_(default_brain_config()),
      ui_settings_(default_ui_settings()),
      api_settings_(default_api_settings()),
      log_level_("INFO") {}

bool ConfigManager::load_config(const std::string& filepath) {
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
    // Simplified JSON parsing
    return !json_content.empty();
}

std::string ConfigManager::to_json() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"brain\": {\n";
    ss << "    \"num_layers\": " << brain_config_.num_layers << ",\n";
    ss << "    \"neurons_per_layer\": " << brain_config_.neurons_per_layer << ",\n";
    ss << "    \"learning_rate\": " << brain_config_.learning_rate << "\n";
    ss << "  },\n";
    ss << "  \"ui\": {\n";
    ss << "    \"dark_mode\": " << (ui_settings_.dark_mode ? "true" : "false") << ",\n";
    ss << "    \"window_width\": " << ui_settings_.window_width << ",\n";
    ss << "    \"window_height\": " << ui_settings_.window_height << "\n";
    ss << "  },\n";
    ss << "  \"api\": {\n";
    ss << "    \"host\": \"" << api_settings_.host << "\",\n";
    ss << "    \"port\": " << api_settings_.port << "\n";
    ss << "  }\n";
    ss << "}\n";
    return ss.str();
}

} // namespace BrainLLM
