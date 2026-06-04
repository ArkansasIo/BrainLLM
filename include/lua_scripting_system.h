#pragma once

#include <map>
#include <string>
#include <vector>

namespace BrainLLM {

enum class ScriptScope {
    System,
    Brain,
    Dialogue,
    Audio,
    User
};

struct LuaScriptDefinition {
    std::string script_id;
    std::string name;
    std::string script_path;
    std::string entry_point = "main";
    ScriptScope scope = ScriptScope::User;
    std::vector<std::string> sub_scripts;
    bool enabled = true;
};

struct ScriptExecutionResult {
    bool success = false;
    std::string script_id;
    std::string output;
    std::string error;
};

class LuaScriptingSystem {
public:
    LuaScriptingSystem();
    ~LuaScriptingSystem() = default;

    void register_script(const LuaScriptDefinition& script);
    bool load_script_manifest(const std::string& manifest_path);
    std::vector<LuaScriptDefinition> get_scripts() const;
    std::vector<LuaScriptDefinition> get_scripts_by_scope(ScriptScope scope) const;

    ScriptExecutionResult execute_script(const std::string& script_id,
                                         const std::map<std::string, std::string>& args) const;
    std::string generate_lua_bootstrap() const;

private:
    std::map<std::string, LuaScriptDefinition> scripts_;

    static std::string scope_to_string(ScriptScope scope);
};

} // namespace BrainLLM
