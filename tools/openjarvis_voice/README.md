# OpenJarvis Voice Subset for BrainLLM

This folder vendors the voice-related subset of OpenJarvis:

- `openjarvis/speech`: speech-to-text and text-to-speech backend classes.
- `openjarvis/tools/audio_tool.py`: audio transcription tool.
- `openjarvis/tools/text_to_speech.py`: text-to-speech tool.
- `openjarvis/core`: minimal support modules required by those files.

The original OpenJarvis project is licensed under Apache-2.0. See
`LICENSE.openjarvis`.

## Usage

From this directory:

```powershell
python synthesize.py --backend kokoro --text "BrainLLM is online." --output generated.wav
```

Available backends depend on installed Python packages and API keys:

- `kokoro`: local TTS, requires `kokoro`, `numpy`, and `soundfile`.
- `openai_tts`: cloud TTS, requires `httpx` and `OPENAI_API_KEY`.
- `cartesia`: cloud TTS, requires `httpx` and `CARTESIA_API_KEY`.

BrainLLM's C++ GUI still uses the built-in Windows SAPI path by default. This
vendored subset is an optional packaged voice backend for future integration.
