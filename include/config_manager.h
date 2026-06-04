#pragma once

#include "brain_types.h"
#include <string>
#include <map>

namespace BrainLLM {

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager() = default;
    
    // Configuration loading/saving
    bool load_config(const std::string& filepath);
    bool save_config(const std::string& filepath);
    
    // Brain configuration
    BrainConfig get_brain_config() const;
    void set_brain_config(const BrainConfig& config);
    
    // UI Settings
    struct UISettings {
        bool dark_mode;
        int window_width;
        int window_height;
        bool maximize_on_start;
        float font_size;
        std::string theme;
    };
    
    UISettings get_ui_settings() const;
    void set_ui_settings(const UISettings& settings);
    
    // API Settings
    struct APISettings {
        std::string host;
        int port;
        bool enable_cors;
        int max_connections;
        std::string api_key;
    };
    
    APISettings get_api_settings() const;
    void set_api_settings(const APISettings& settings);
    
    // Logging
    void set_log_level(const std::string& level);
    std::string get_log_level() const;
    
    // Defaults
    static BrainConfig default_brain_config();
    static UISettings default_ui_settings();
    static APISettings default_api_settings();
    
private:
    BrainConfig brain_config_;
    UISettings ui_settings_;
    APISettings api_settings_;
    std::string log_level_;
    
    bool parse_json(const std::string& json_content);
    std::string to_json() const;
};

} // namespace BrainLLM
