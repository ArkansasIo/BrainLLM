"""Small BrainLLM wrapper around the vendored OpenJarvis TTS backends."""

from __future__ import annotations

import argparse
from pathlib import Path


def synthesize_text(
    text: str,
    output: Path,
    *,
    backend: str,
    voice_id: str = "",
    speed: float = 1.0,
    output_format: str = "",
) -> Path:
    import openjarvis.speech  # noqa: F401 - registers vendored backends
    from openjarvis.core.registry import TTSRegistry

    aliases = {"openai": "openai_tts"}
    backend_key = aliases.get(backend, backend)
    if not TTSRegistry.contains(backend_key):
        available = ", ".join(TTSRegistry.keys()) or "none"
        raise RuntimeError(f"TTS backend '{backend_key}' is not available. Available: {available}")

    backend_cls = TTSRegistry.get(backend_key)
    result_format = output_format or output.suffix.lstrip(".") or "wav"
    result = backend_cls().synthesize(
        text,
        voice_id=voice_id,
        speed=speed,
        output_format=result_format,
    )

    output.parent.mkdir(parents=True, exist_ok=True)
    result.save(output)
    return output


def main() -> int:
    parser = argparse.ArgumentParser(description="Synthesize speech with vendored OpenJarvis TTS.")
    parser.add_argument("--text", required=True, help="Text to synthesize.")
    parser.add_argument("--output", required=True, help="Output audio path.")
    parser.add_argument("--backend", default="kokoro", help="TTS backend: kokoro, openai_tts, cartesia.")
    parser.add_argument("--voice-id", default="", help="Backend-specific voice identifier.")
    parser.add_argument("--speed", type=float, default=1.0, help="Speech speed multiplier.")
    parser.add_argument("--format", default="", help="Output format. Defaults to output extension.")
    args = parser.parse_args()

    output = synthesize_text(
        args.text,
        Path(args.output),
        backend=args.backend,
        voice_id=args.voice_id,
        speed=args.speed,
        output_format=args.format,
    )
    print(output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
