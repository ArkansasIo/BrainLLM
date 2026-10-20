# BrainLLM Personality System

The personality layer is a configurable behavior policy between conversation context and model generation. It changes tone, priorities, response structure, and interaction strategy; it does not replace the underlying model.

## Built-in personalities

| Profile | Behavior |
|---|---|
| Balanced | General-purpose, clear and helpful |
| Analytical | Evidence-driven, technical and systematic |
| Creative | Exploratory, imaginative and narrative |
| Empathetic | Supportive, emotionally aware and respectful |
| Assertive | Direct, decisive and action-oriented |
| Curious Explorer | Question-driven and exploratory |
| Stoic | Calm, concise and disciplined |
| Playful | Light, engaging and humorous |
| Mentor | Teaching-oriented and Socratic |
| Companion | Warm and conversational |
| Researcher | Methodical and evidence-focused |
| Commander | Priority-driven and execution-focused |

## Architecture

`PersonalityProfile` stores stable traits and preferences. `PersonalityContext` carries per-request context such as technical-task state and emotional intensity. `PersonalityEngine` converts both into a `PersonalityResponse` and system instruction.

This separation allows future adapters for GUI controls, REST APIs, database persistence, model routing, and per-user profiles without coupling personality logic to a specific LLM provider.

## Extension points

1. Add new profiles through `registerProfile()`.
2. Add persistence by loading `data/personality_profiles.json`.
3. Add per-user active profile IDs.
4. Add conversation-state modifiers for temporary mood or role.
5. Add safety and policy checks after instruction generation and before model execution.
6. Add evaluation tests for deterministic tone and instruction selection.
