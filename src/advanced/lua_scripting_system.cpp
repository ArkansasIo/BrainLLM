#include "lua_scripting_system.h"

#include <fstream>
#include <sstream>

namespace BrainLLM {

LuaScriptingSystem::LuaScriptingSystem() {
    register_script({
        "system_bootstrap",
        "System Bootstrap",
        "scripts/lua/system/bootstrap.lua",
        "main",
        ScriptScope::System,
        {"scripts/lua/subscripts/common.lua"},
        true
    });

    register_script({
        "voice_woman_startup",
        "Woman Voice Speak",
        "scripts/lua/audio/woman_voice.lua",
        "main",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_synthesize",
        "Woman Voice Synthesize WAV",
        "scripts/lua/audio/woman_voice.lua",
        "synthesize",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_recognize",
        "Woman Voice Recognize Microphone",
        "scripts/lua/audio/woman_voice.lua",
        "recognize",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_startup_tone",
        "Woman Voice Startup Tone",
        "scripts/lua/audio/woman_voice.lua",
        "startup_tone",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_notify_tone",
        "Woman Voice Notify Tone",
        "scripts/lua/audio/woman_voice.lua",
        "notify_tone",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_listen",
        "Woman Voice Listen",
        "scripts/lua/audio/woman_voice.lua",
        "listen",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_reply",
        "Woman Voice Dialogue Reply",
        "scripts/lua/audio/woman_voice.lua",
        "reply",
        ScriptScope::Dialogue,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_startup_sequence",
        "Woman Voice Startup Sequence",
        "scripts/lua/audio/woman_voice.lua",
        "startup_sequence",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_notification",
        "Woman Voice Notification",
        "scripts/lua/audio/woman_voice.lua",
        "notification",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_error",
        "Woman Voice Error Message",
        "scripts/lua/audio/woman_voice.lua",
        "error_message",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });

    register_script({
        "voice_woman_status",
        "Woman Voice Status Message",
        "scripts/lua/audio/woman_voice.lua",
        "status",
        ScriptScope::Audio,
        {"scripts/lua/subscripts/audio_helpers.lua"},
        true
    });
}

void LuaScriptingSystem::register_script(const LuaScriptDefinition& script) {
    scripts_[script.script_id] = script;
}

bool LuaScriptingSystem::load_script_manifest(const std::string& manifest_path) {
    std::ifstream file(manifest_path);
    return file.good();
}

std::vector<LuaScriptDefinition> LuaScriptingSystem::get_scripts() const {
    std::vector<LuaScriptDefinition> result;
    for (const auto& pair : scripts_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<LuaScriptDefinition> LuaScriptingSystem::get_scripts_by_scope(ScriptScope scope) const {
    std::vector<LuaScriptDefinition> result;
    for (const auto& pair : scripts_) {
        if (pair.second.scope == scope) {
            result.push_back(pair.second);
        }
    }
    return result;
}

ScriptExecutionResult LuaScriptingSystem::execute_script(
    const std::string& script_id,
    const std::map<std::string, std::string>& args) const {
    ScriptExecutionResult result;
    result.script_id = script_id;

    auto it = scripts_.find(script_id);
    if (it == scripts_.end()) {
        result.error = "Script not registered: " + script_id;
        return result;
    }

    if (!it->second.enabled) {
        result.error = "Script disabled: " + script_id;
        return result;
    }

    const auto arg_or = [&args](const std::string& key, const std::string& fallback) {
        auto arg = args.find(key);
        return arg == args.end() || arg->second.empty() ? fallback : arg->second;
    };

    std::string action = "script_plan";
    std::string endpoint;
    if (it->second.entry_point == "main" || it->second.entry_point == "speak" ||
        it->second.entry_point == "reply" || it->second.entry_point == "notification" ||
        it->second.entry_point == "error_message" || it->second.entry_point == "status") {
        action = "speak";
        endpoint = "/api/speech/speak";
    } else if (it->second.entry_point == "synthesize") {
        action = "synthesize";
        endpoint = "/api/speech/synthesize";
    } else if (it->second.entry_point == "recognize" || it->second.entry_point == "listen") {
        action = "recognize";
        endpoint = "/api/speech/recognize";
    } else if (it->second.entry_point == "startup_sequence") {
        action = "sequence";
        endpoint = "/api/speech/speak";
    } else if (it->second.entry_point == "startup_tone" || it->second.entry_point == "notify_tone") {
        action = "tone";
    }

    std::ostringstream output;
    output << "{"
           << "\"script_id\":\"" << it->second.script_id << "\","
           << "\"path\":\"" << it->second.script_path << "\","
           << "\"entry_point\":\"" << it->second.entry_point << "\","
           << "\"action\":\"" << action << "\"";
    if (!endpoint.empty()) {
        output << ",\"api\":\"" << endpoint << "\"";
    }
    output << ",\"profile\":\"woman_default\"";
    output << ",\"text\":\"" << arg_or("text", arg_or("assistant_text", "BrainLLM is ready.")) << "\"";
    output << ",\"output_path\":\"" << arg_or("output_path", "assets/audio/woman/generated_response.wav") << "\"";
    output << ",\"timeout_seconds\":\"" << arg_or("timeout_seconds", "6") << "\"";
    output << ",\"args\":{";
    bool first_arg = true;
    for (const auto& arg : args) {
        if (!first_arg) {
            output << ",";
        }
        output << "\"" << arg.first << "\":\"" << arg.second << "\"";
        first_arg = false;
    }
    output << "}}";

    result.success = true;
    result.output = output.str();
    return result;
}

std::string LuaScriptingSystem::generate_lua_bootstrap() const {
    std::ostringstream lua;
    lua << "-- BrainLLM Lua bootstrap\n";
    lua << "BrainLLM = BrainLLM or {}\n";
    lua << "BrainLLM.scripts = {}\n\n";

    for (const auto& pair : scripts_) {
        const auto& script = pair.second;
        lua << "BrainLLM.scripts['" << script.script_id << "'] = {\n";
        lua << "  name = '" << script.name << "',\n";
        lua << "  path = '" << script.script_path << "',\n";
        lua << "  entry = '" << script.entry_point << "',\n";
        lua << "  scope = '" << scope_to_string(script.scope) << "',\n";
        lua << "  enabled = " << (script.enabled ? "true" : "false") << "\n";
        lua << "}\n\n";
    }

    return lua.str();
}

std::string LuaScriptingSystem::scope_to_string(ScriptScope scope) {
    switch (scope) {
        case ScriptScope::System:
            return "system";
        case ScriptScope::Brain:
            return "brain";
        case ScriptScope::Dialogue:
            return "dialogue";
        case ScriptScope::Audio:
            return "audio";
        case ScriptScope::User:
            return "user";
        default:
            return "user";
    }
}

} // namespace BrainLLM
