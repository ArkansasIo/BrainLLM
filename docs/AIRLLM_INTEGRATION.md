# AIRLLM Integration

BrainLLM vendors the public AIRLLM source under `third_party/airllm`.

AIRLLM repository: https://github.com/lyogavin/airllm  
License: Apache-2.0

## Purpose

AIRLLM is a Python runtime for memory-optimized LLM inference. Its README describes running large models by loading/splitting model layers so inference can run on lower-memory hardware.

BrainLLM does not compile AIRLLM into the C++ binaries. Instead, it provides:

- `AirLLMBridge` in `include/advanced_architectures.h`
- `tools/airllm_runner.py` for optional Python inference
- Vendored AIRLLM source in `third_party/airllm`

## C++ Usage

```cpp
AirLLMRuntimeConfig config;
config.model_id = "garage-bAInd/Platypus2-70B-instruct";
config.compression = "4bit";
config.max_new_tokens = 64;

AirLLMBridge bridge(config);
if (bridge.is_source_available()) {
    std::string command = bridge.build_inference_command(
        "What is BrainLLM?",
        "build/airllm_output.json"
    );
}
```

`ComprehensiveBrain` also exposes:

```cpp
brain.get_airllm_bridge();
```

## Python Runtime Setup

AIRLLM dependencies are intentionally not installed automatically because they can be large and hardware-specific.

```bash
python -m pip install -r third_party/airllm/requirements.txt
```

Then run:

```bash
python tools/airllm_runner.py ^
  --airllm-root third_party/airllm ^
  --model garage-bAInd/Platypus2-70B-instruct ^
  --prompt "What is the capital of the United States?" ^
  --output build/airllm_output.json ^
  --max-new-tokens 32
```

Notes:

- Large models can require substantial disk space for Hugging Face cache and layer shards.
- Some models require Hugging Face access approval or tokens.
- CUDA behavior depends on your local PyTorch/GPU installation.
