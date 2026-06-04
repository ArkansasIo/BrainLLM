#!/usr/bin/env python3
"""Run AIRLLM from the vendored third_party source tree.

This script is intentionally small: BrainLLM remains a C++ project, while
AIRLLM stays an optional Python runtime dependency.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="BrainLLM AIRLLM runner")
    parser.add_argument("--airllm-root", default="third_party/airllm")
    parser.add_argument("--model", required=True)
    parser.add_argument("--prompt", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--max-new-tokens", type=int, default=128)
    parser.add_argument("--layer-shards-path", default="")
    parser.add_argument("--compression", default="")
    parser.add_argument("--cpu", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    airllm_root = Path(args.airllm_root).resolve()
    package_root = airllm_root / "air_llm"

    if not package_root.exists():
        raise FileNotFoundError(f"AIRLLM package root not found: {package_root}")

    sys.path.insert(0, str(package_root))

    try:
        import torch
        from airllm import AutoModel
    except Exception as exc:  # pragma: no cover - depends on optional Python deps
        raise RuntimeError(
            "AIRLLM runtime dependencies are missing. Install dependencies from "
            "third_party/airllm/requirements.txt before running inference."
        ) from exc

    model_kwargs = {}
    if args.layer_shards_path:
        model_kwargs["layer_shards_saving_path"] = args.layer_shards_path
    if args.compression:
        model_kwargs["compression"] = args.compression

    model = AutoModel.from_pretrained(args.model, **model_kwargs)
    input_tokens = model.tokenizer(
        [args.prompt],
        return_tensors="pt",
        return_attention_mask=False,
        truncation=True,
        padding=False,
    )

    input_ids = input_tokens["input_ids"]
    if not args.cpu and torch.cuda.is_available():
        input_ids = input_ids.cuda()

    generation_output = model.generate(
        input_ids,
        max_new_tokens=args.max_new_tokens,
        use_cache=True,
        return_dict_in_generate=True,
    )
    text = model.tokenizer.decode(generation_output.sequences[0])

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        json.dumps(
            {
                "model": args.model,
                "prompt": args.prompt,
                "text": text,
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    print(text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
