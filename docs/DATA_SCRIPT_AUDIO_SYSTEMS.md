# Data, Lua, and Audio Systems

This project now includes dependency-free infrastructure for persistent data layouts, script orchestration, and voice/audio assets.

## Database and Spreadsheet Layout

Files:

- `include/data_config_system.h`
- `src/settings/data_config_system.cpp`
- `data/schema/brainllm_schema.sql`
- `data/spreadsheets/brainllm_layout.csv`

The SQL file is SQLite-compatible. The CSV file is an Excel-compatible spreadsheet layout listing every table, field, type, primary-key flag, nullable flag, and default value.

Default tables:

- `brain_objects`
- `memory_records`
- `conversation_turns`
- `personality_profiles`
- `lua_scripts`
- `audio_assets`
- `safety_audit_log`

## Lua Scripting

Files:

- `include/lua_scripting_system.h`
- `src/advanced/lua_scripting_system.cpp`
- `scripts/lua/system/bootstrap.lua`
- `scripts/lua/audio/woman_voice.lua`
- `scripts/lua/subscripts/common.lua`
- `scripts/lua/subscripts/audio_helpers.lua`

The C++ module registers scripts and subscripts and can generate a Lua bootstrap manifest. It does not embed a Lua VM yet, so the project remains buildable without Lua development packages.

## Woman Voice and Audio

Files:

- `include/voice_audio_system.h`
- `src/advanced/voice_audio_system.cpp`
- `assets/audio/woman/woman_voice_profile.json`
- `assets/audio/woman/startup_tone.wav`
- `assets/audio/woman/notify_tone.wav`

The checked-in WAV files provide bundled UI and startup tones. Spoken text output uses the platform TTS backend through `VoiceAudioSystem` when it is available.
