# BrainLLM Developer Roadmap

This page separates repository capabilities, implemented integration points, and recommended next work. A documented subsystem is not necessarily a production-grade or independently trained language model.

## Current architecture

- **C++17 / CMake / Qt 6** — desktop GUI and API executables.
- **Core engine** — neural-network primitives, memory, attention, language-engine interfaces, and configuration.
- **Advanced modules** — dialogue personality, safety/security, learning, robotics, Lua scripting, and voice/audio.
- **Cognitive modules** — cognitive processes, English processing, multilingual processing, grammar, and integrated cognition.
- **Quantum demonstration modules** — quantum circuit primitives and quantum-inspired text/search integrations. Treat these as simulations unless validated against appropriate quantum hardware.
- **REST interface** — Qt network server with chat, processing, generation, health/status, memory, configuration, training, reset, and optional runtime bridges.
- **Browser client and Qt GUI** — user-facing ways to interact with the runtime.
- **Data and extension assets** — personality profile JSON, SQL schema, CSV layout, Lua scripts, voice assets, and optional vendor integrations.

## Backend feature inventory

| Area | Current entry point | Recommended next hardening |
|---|---|---|
| Build and packaging | `CMakeLists.txt`, build scripts | Verify clean builds on Windows and Linux; check optional paths |
| Core inference | `include/llm_engine.h`, `src/brain/llm_engine.cpp` | Define provider interface, cancellation, limits, and deterministic tests |
| Memory | `include/memory_system.h` | Specify persistence, deletion, size limits, and retrieval ranking |
| Personality | `include/dialogue_personality.h`, `data/personality_profiles.json` | Validate profile schema, test name aliases, persist selected profile |
| REST API | `include/rest_server.h`, `src/api/` | Request-size/time limits, input validation, structured errors, API auth for non-local exposure |
| Configuration | `config.ini`, settings modules | Centralize validation and document precedence/defaults |
| Safety | `include/safety_security.h` and implementation | Test boundary handling and ensure policies run before model execution |
| Scripting | Lua module and `scripts/lua/` | Capability restrictions, execution timeouts, and clear script error reporting |
| Voice | `include/voice_audio_system.h`, `assets/audio/` | Device selection, codec validation, and graceful missing-device behavior |
| Data contracts | `data/schema/`, `data/spreadsheets/` | Migration/version strategy and integrity tests |
| Optional model bridges | `tools/`, `third_party/` | Detect unavailable dependencies and report setup requirements clearly |

## Frontend and desktop experience

- IDE-style editor, console, chat, settings, menus, and visualization panels.
- Browser client served by the API application.
- Model/runtime status and system metrics.
- Voice/audio controls and personality selection should expose clear availability and error states.
- Prefer responsive layouts, accessible labels, keyboard navigation, and explicit loading/empty/error states.

## Recommended implementation sequence

### P0 — reliable baseline
1. Keep optional vendor directories optional during the post-build copy stage.
2. Make CI build and run smoke tests on a clean checkout.
3. Validate `data/personality_profiles.json` (version, required fields, unique IDs, supported profile kinds).
4. Add targeted unit tests for personality profile lookup, unknown profiles, trait clamping, and feedback updates.
5. Document actual supported endpoints and build requirements from source.

### P1 — safer runtime
1. Establish API request limits and consistent JSON errors.
2. Avoid exposing a non-local API listener without an explicit security configuration.
3. Add cancellation and timeouts for external model runners.
4. Add configuration validation and actionable missing-dependency messages.
5. Add structured logging and redact secrets from logs.

### P2 — durable developer platform
1. Versioned provider interfaces for local and external inference engines.
2. Per-user conversation and profile persistence with explicit retention controls.
3. Plugin registry with manifest validation, capability declarations, and compatibility versions.
4. Integration tests for GUI-to-engine and browser-to-API workflows.
5. Reproducible releases and platform build artifacts.

## Validation checklist

- [ ] Configure from a fresh checkout with documented Qt 6 dependencies.
- [ ] Build both `BrainLLM_GUI` and `BrainLLM_API`.
- [ ] Run `ctest --test-dir build --output-on-failure`.
- [ ] Validate the personality profile JSON against the documented schema.
- [ ] Confirm absent optional vendor directories do not fail the build.
- [ ] Verify API health/status and invalid-request responses.
- [ ] Test with no optional model provider configured.
- [ ] Confirm README examples match the actual request parser and endpoint behavior.

## Important scope note

BrainLLM is an extensible simulation and application framework. Features described as quantum, cognitive, or brain-inspired should not be represented as evidence of human-level cognition or a production LLM unless supported by reproducible evaluations.
