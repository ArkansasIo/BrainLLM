#pragma once

#include <map>
#include <string>
#include <vector>

namespace BrainLLM {

enum class DataFieldType {
    Integer,
    Real,
    Text,
    Boolean,
    Timestamp,
    Json
};

struct DataField {
    std::string name;
    DataFieldType type;
    bool primary_key = false;
    bool nullable = true;
    std::string default_value;
};

struct DataTableSchema {
    std::string name;
    std::vector<DataField> fields;
    std::vector<std::string> indexes;
};

struct DatabaseLayoutConfig {
    std::string database_path = "data/brainllm.db";
    std::string spreadsheet_export_dir = "data/spreadsheets";
    std::string sql_dialect = "sqlite";
    bool auto_migrate = true;
    bool export_csv_on_shutdown = false;
};

class DataConfigSystem {
public:
    DataConfigSystem();
    ~DataConfigSystem() = default;

    void set_config(const DatabaseLayoutConfig& config);
    DatabaseLayoutConfig get_config() const;

    void register_table(const DataTableSchema& table);
    std::vector<DataTableSchema> get_tables() const;

    std::string generate_sql_schema() const;
    bool write_sql_schema(const std::string& filepath) const;
    bool write_spreadsheet_layout_csv(const std::string& filepath) const;

    static std::vector<DataTableSchema> default_brain_object_tables();

private:
    DatabaseLayoutConfig config_;
    std::vector<DataTableSchema> tables_;

    static std::string sql_type(DataFieldType type);
    static std::string csv_escape(const std::string& value);
};

} // namespace BrainLLM
