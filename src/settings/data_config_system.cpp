#include "data_config_system.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace BrainLLM {

DataConfigSystem::DataConfigSystem()
    : tables_(default_brain_object_tables()) {
}

void DataConfigSystem::set_config(const DatabaseLayoutConfig& config) {
    config_ = config;
}

DatabaseLayoutConfig DataConfigSystem::get_config() const {
    return config_;
}

void DataConfigSystem::register_table(const DataTableSchema& table) {
    tables_.push_back(table);
}

std::vector<DataTableSchema> DataConfigSystem::get_tables() const {
    return tables_;
}

std::string DataConfigSystem::generate_sql_schema() const {
    std::ostringstream sql;
    sql << "-- BrainLLM database layout (" << config_.sql_dialect << ")\n";
    sql << "PRAGMA foreign_keys = ON;\n\n";

    for (const auto& table : tables_) {
        sql << "CREATE TABLE IF NOT EXISTS " << table.name << " (\n";
        for (size_t i = 0; i < table.fields.size(); ++i) {
            const auto& field = table.fields[i];
            sql << "  " << field.name << " " << sql_type(field.type);
            if (field.primary_key) {
                sql << " PRIMARY KEY";
            }
            if (!field.nullable) {
                sql << " NOT NULL";
            }
            if (!field.default_value.empty()) {
                sql << " DEFAULT " << field.default_value;
            }
            if (i + 1 < table.fields.size()) {
                sql << ",";
            }
            sql << "\n";
        }
        sql << ");\n";

        for (const auto& index : table.indexes) {
            sql << "CREATE INDEX IF NOT EXISTS idx_" << table.name << "_"
                << index << " ON " << table.name << "(" << index << ");\n";
        }
        sql << "\n";
    }

    return sql.str();
}

bool DataConfigSystem::write_sql_schema(const std::string& filepath) const {
    std::filesystem::path path(filepath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    file << generate_sql_schema();
    return true;
}

bool DataConfigSystem::write_spreadsheet_layout_csv(const std::string& filepath) const {
    std::filesystem::path path(filepath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    file << "table,field,type,primary_key,nullable,default_value\n";
    for (const auto& table : tables_) {
        for (const auto& field : table.fields) {
            file << csv_escape(table.name) << ","
                 << csv_escape(field.name) << ","
                 << csv_escape(sql_type(field.type)) << ","
                 << (field.primary_key ? "true" : "false") << ","
                 << (field.nullable ? "true" : "false") << ","
                 << csv_escape(field.default_value) << "\n";
        }
    }

    return true;
}

std::vector<DataTableSchema> DataConfigSystem::default_brain_object_tables() {
    return {
        {
            "brain_objects",
            {
                {"object_id", DataFieldType::Text, true, false, ""},
                {"object_type", DataFieldType::Text, false, false, ""},
                {"name", DataFieldType::Text, false, false, ""},
                {"payload_json", DataFieldType::Json, false, true, ""},
                {"created_at", DataFieldType::Timestamp, false, false, "CURRENT_TIMESTAMP"}
            },
            {"object_type", "created_at"}
        },
        {
            "memory_records",
            {
                {"memory_id", DataFieldType::Text, true, false, ""},
                {"content", DataFieldType::Text, false, false, ""},
                {"importance", DataFieldType::Real, false, false, "0.0"},
                {"category", DataFieldType::Text, false, true, ""},
                {"timestamp", DataFieldType::Timestamp, false, false, "CURRENT_TIMESTAMP"}
            },
            {"category", "timestamp"}
        },
        {
            "conversation_turns",
            {
                {"turn_id", DataFieldType::Text, true, false, ""},
                {"speaker", DataFieldType::Text, false, false, ""},
                {"message", DataFieldType::Text, false, false, ""},
                {"response", DataFieldType::Text, false, true, ""},
                {"confidence", DataFieldType::Real, false, false, "0.0"},
                {"context", DataFieldType::Text, false, true, ""}
            },
            {"speaker"}
        },
        {
            "personality_profiles",
            {
                {"profile_id", DataFieldType::Text, true, false, ""},
                {"framework", DataFieldType::Text, false, false, ""},
                {"type_code", DataFieldType::Text, false, false, ""},
                {"traits_json", DataFieldType::Json, false, false, ""},
                {"response_style", DataFieldType::Text, false, true, ""}
            },
            {"framework", "type_code"}
        },
        {
            "lua_scripts",
            {
                {"script_id", DataFieldType::Text, true, false, ""},
                {"name", DataFieldType::Text, false, false, ""},
                {"entry_point", DataFieldType::Text, false, false, ""},
                {"script_path", DataFieldType::Text, false, false, ""},
                {"enabled", DataFieldType::Boolean, false, false, "1"}
            },
            {"name", "enabled"}
        },
        {
            "audio_assets",
            {
                {"asset_id", DataFieldType::Text, true, false, ""},
                {"voice_profile", DataFieldType::Text, false, false, ""},
                {"file_path", DataFieldType::Text, false, false, ""},
                {"sample_rate", DataFieldType::Integer, false, false, "44100"},
                {"duration_ms", DataFieldType::Integer, false, false, "0"}
            },
            {"voice_profile"}
        },
        {
            "safety_audit_log",
            {
                {"audit_id", DataFieldType::Text, true, false, ""},
                {"event_type", DataFieldType::Text, false, false, ""},
                {"decision", DataFieldType::Text, false, false, ""},
                {"reasoning", DataFieldType::Text, false, true, ""},
                {"created_at", DataFieldType::Timestamp, false, false, "CURRENT_TIMESTAMP"}
            },
            {"event_type", "created_at"}
        }
    };
}

std::string DataConfigSystem::sql_type(DataFieldType type) {
    switch (type) {
        case DataFieldType::Integer:
            return "INTEGER";
        case DataFieldType::Real:
            return "REAL";
        case DataFieldType::Text:
            return "TEXT";
        case DataFieldType::Boolean:
            return "INTEGER";
        case DataFieldType::Timestamp:
            return "DATETIME";
        case DataFieldType::Json:
            return "TEXT";
        default:
            return "TEXT";
    }
}

std::string DataConfigSystem::csv_escape(const std::string& value) {
    std::string escaped = "\"";
    for (char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
        } else {
            escaped += ch;
        }
    }
    escaped += "\"";
    return escaped;
}

} // namespace BrainLLM
