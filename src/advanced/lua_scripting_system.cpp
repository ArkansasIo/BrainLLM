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
        "Woman Voice Startup",
        "scripts/lua/audio/woman_voice.lua",
        "main",
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

    std::ostringstream output;
    output << "Lua script planned: " << it->second.script_path
           << "::" << it->second.entry_point;
    for (const auto& arg : args) {
        output << " " << arg.first << "=" << arg.second;
    }

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
