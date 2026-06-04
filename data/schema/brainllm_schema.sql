-- BrainLLM default SQLite database layout
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS brain_objects (
  object_id TEXT PRIMARY KEY NOT NULL,
  object_type TEXT NOT NULL,
  name TEXT NOT NULL,
  payload_json TEXT,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE INDEX IF NOT EXISTS idx_brain_objects_object_type ON brain_objects(object_type);
CREATE INDEX IF NOT EXISTS idx_brain_objects_created_at ON brain_objects(created_at);

CREATE TABLE IF NOT EXISTS memory_records (
  memory_id TEXT PRIMARY KEY NOT NULL,
  content TEXT NOT NULL,
  importance REAL NOT NULL DEFAULT 0.0,
  category TEXT,
  timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE INDEX IF NOT EXISTS idx_memory_records_category ON memory_records(category);
CREATE INDEX IF NOT EXISTS idx_memory_records_timestamp ON memory_records(timestamp);

CREATE TABLE IF NOT EXISTS conversation_turns (
  turn_id TEXT PRIMARY KEY NOT NULL,
  speaker TEXT NOT NULL,
  message TEXT NOT NULL,
  response TEXT,
  confidence REAL NOT NULL DEFAULT 0.0,
  context TEXT
);
CREATE INDEX IF NOT EXISTS idx_conversation_turns_speaker ON conversation_turns(speaker);

CREATE TABLE IF NOT EXISTS personality_profiles (
  profile_id TEXT PRIMARY KEY NOT NULL,
  framework TEXT NOT NULL,
  type_code TEXT NOT NULL,
  traits_json TEXT NOT NULL,
  response_style TEXT
);
CREATE INDEX IF NOT EXISTS idx_personality_profiles_framework ON personality_profiles(framework);
CREATE INDEX IF NOT EXISTS idx_personality_profiles_type_code ON personality_profiles(type_code);

CREATE TABLE IF NOT EXISTS lua_scripts (
  script_id TEXT PRIMARY KEY NOT NULL,
  name TEXT NOT NULL,
  entry_point TEXT NOT NULL,
  script_path TEXT NOT NULL,
  enabled INTEGER NOT NULL DEFAULT 1
);
CREATE INDEX IF NOT EXISTS idx_lua_scripts_name ON lua_scripts(name);
CREATE INDEX IF NOT EXISTS idx_lua_scripts_enabled ON lua_scripts(enabled);

CREATE TABLE IF NOT EXISTS audio_assets (
  asset_id TEXT PRIMARY KEY NOT NULL,
  voice_profile TEXT NOT NULL,
  file_path TEXT NOT NULL,
  sample_rate INTEGER NOT NULL DEFAULT 44100,
  duration_ms INTEGER NOT NULL DEFAULT 0
);
CREATE INDEX IF NOT EXISTS idx_audio_assets_voice_profile ON audio_assets(voice_profile);

CREATE TABLE IF NOT EXISTS safety_audit_log (
  audit_id TEXT PRIMARY KEY NOT NULL,
  event_type TEXT NOT NULL,
  decision TEXT NOT NULL,
  reasoning TEXT,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);
CREATE INDEX IF NOT EXISTS idx_safety_audit_log_event_type ON safety_audit_log(event_type);
CREATE INDEX IF NOT EXISTS idx_safety_audit_log_created_at ON safety_audit_log(created_at);
