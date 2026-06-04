# BrainLLM - AI Brain Simulator

A comprehensive C++ application simulating an LLM-based human brain with a sophisticated Windows GUI, REST API, IDE-like interface with menus, settings, and advanced visualization - inspired by the movie Chappie.

## Features

### Core Engine
- **Neural Network**: Multi-layer neural network with configurable architecture
- **LLM Engine**: Language model processing and generation
- **Memory System**: Persistent and decaying memory management
- **Attention Mechanism**: Multi-head attention for focused processing

### GUI Components
- **IDE-like Interface**: Professional editor and console
- **Real-time Visualization**: Neural network activity visualization
- **Menu System**: Comprehensive menus with submenus
- **Settings Dialog**: Configure brain, UI, and API settings
- **Dockable Widgets**: Flexible layout with memory and visualization panels

### REST API
- `POST /api/process` - Process input text
- `POST /api/generate` - Generate responses
- `POST /api/chat` - Chat endpoint used by the browser client
- `GET /api/status` - Get metrics and status
- `GET /api/health` - Health/status alias
- `GET /api/memory` - Query memory
- `GET /api/config` - Get configuration
- `GET /api/endpoints` - List available endpoints
- `GET /api/algorithms` - List active local LLM algorithms
- `POST /api/train` - Train the model
- `POST /api/reset` - Reset state, context, and memory
- `GET/POST /api/airllm/config` - Read or update AirLLM runtime settings
- `GET /client` - Browser-based frontend client

### Settings & Configuration
- Brain parameters (layers, neurons, learning rate)
- UI customization (dark mode, themes, fonts)
- API configuration (host, port, CORS)
- Logging levels

## Build Instructions

### Requirements
- C++17 or later
- CMake 3.16+
- Qt6 (Core, Gui, Widgets, Network, Concurrent)
- Windows 10 or later

### Building

```bash
cd C:\BrainLLM
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The Windows and Linux/macOS build scripts also package runnable files into `output/build`.

### Running

**GUI Application:**
```bash
.\Release\BrainLLM_GUI.exe
```

**API Server:**
```bash
.\Release\BrainLLM_API.exe
```

**Browser Client:**
```text
http://localhost:8080/client
```

## Project Structure

```text
BrainLLM/
|-- include/
|   |-- brain_types.h                  # Core type definitions
|   |-- neural_network.h               # Neural network interface
|   |-- memory_system.h                # Memory management
|   |-- attention_mechanism.h          # Attention mechanism
|   |-- llm_engine.h                   # Main LLM engine
|   |-- config_manager.h               # Configuration management
|   |-- data_config_system.h           # SQL/spreadsheet layouts
|   |-- advanced_architectures.h       # Transformer/LSTM/Seq2Seq/AIRLLM bridge
|   |-- positronic_neural_network.h    # AGI/deep-search synthesis
|   |-- lua_scripting_system.h         # Lua script registry
|   |-- voice_audio_system.h           # Voice/audio assets
|   |-- rest_server.h                  # REST API server
|   |-- request_handler.h              # API request handling
|   |-- main_window.h                  # Main GUI window
|   |-- editor_widget.h                # Code editor widget
|   |-- console_widget.h               # Console output widget
|   |-- visualization_widget.h         # Neural visualization
|   |-- menu_system.h                  # Menu definitions
|   `-- settings_dialog.h              # Settings UI
|-- src/
|   |-- brain/                         # Core engine implementations
|   |-- settings/                      # Config and data layout systems
|   |-- advanced/                      # Advanced AI modules
|   |-- human_brain/                   # Cognitive and linguistic modules
|   |-- api/                           # REST API implementation
|   |-- main_gui.cpp                   # GUI entry point
|   `-- main_api.cpp                   # API server entry point
|-- gui/                               # GUI widget implementations
|-- web/client/                        # Browser UI served by the API executable
|-- scripts/lua/                       # Lua scripts and subscripts
|-- data/schema/                       # SQL database layout
|-- data/spreadsheets/                 # Excel-compatible CSV layouts
|-- assets/audio/woman/                # Woman voice profile and WAV assets
|-- third_party/airllm/                # Optional AIRLLM source clone
|-- tools/airllm_runner.py             # Optional AIRLLM runner
|-- CMakeLists.txt                     # Build configuration
|-- package.json                       # Project metadata
`-- README.md                          # This file
```
## Usage Examples

### GUI Application

1. Launch `BrainLLM_GUI.exe`
2. Type input in the editor panel
3. Click "Process Input" to process text
4. Click "Generate Response" for LLM generation
5. Use "Train" to improve the model
6. Access Settings -> Preferences for configuration
7. Click "Start API Server" to launch REST API
8. Monitor real-time visualization in the right panel

### API Usage

```bash
# Process input
curl -X POST http://localhost:8080/api/process -d "Hello, World!"

# Generate response
curl -X POST http://localhost:8080/api/generate -d "What is artificial intelligence?"

# Get status
curl http://localhost:8080/api/status

# Chat through the client endpoint
curl -X POST http://localhost:8080/api/chat ^
  -H "Content-Type: application/json" ^
  -d "{\"messages\":[{\"role\":\"user\",\"content\":\"summarize BrainLLM\"}],\"max_tokens\":160}"

# Get memory
curl http://localhost:8080/api/memory?query=learning

# Configure AirLLM at runtime
curl -X POST http://localhost:8080/api/airllm/config ^
  -H "Content-Type: application/json" ^
  -d "{\"model_id\":\"your-model-id\",\"python_executable\":\"python\",\"max_new_tokens\":128,\"use_cuda\":true}"

# Train model
curl -X POST http://localhost:8080/api/train -d "training data"
```

## Menus

### File
- New
- Open
- Save
- Exit

### Edit
- Undo/Redo
- Cut/Copy/Paste

### View
- Zoom controls
- Full screen
- Layout options

### Brain
- Initialize
- Reset
- Process Input
- Generate Response
- Train Model
- View Memory

### Settings
- Preferences
- API Configuration
- Brain Configuration

### Help
- About
- Documentation

## Architecture

### Brain Engine
The core LLM engine consists of:
- **Neural Network**: Layers of neurons with weights and biases
- **Memory System**: Multi-category memory with decay and consolidation
- **Attention Mechanism**: Multi-head attention for context focus
- **Language Processing**: Tokenization, encoding, and generation

### API Server
REST API built with Qt's QTcpServer:
- Thread-safe request handling
- JSON response formatting
- Configurable endpoints
- CORS support

### GUI
Professional IDE-like interface:
- Dockable panels for flexibility
- Real-time visualization
- Responsive menu system
- Settings persistence

## Configuration

Edit settings via the Settings dialog or configuration files:

```json
{
  "brain": {
    "num_layers": 8,
    "neurons_per_layer": 256,
    "learning_rate": 0.001,
    "attention_heads": 8
  },
  "ui": {
    "dark_mode": true,
    "window_width": 1920,
    "window_height": 1080
  },
  "api": {
    "host": "localhost",
    "port": 8080,
    "enable_cors": true
  }
}
```

## Performance Metrics

The application monitors:
- CPU Usage
- Memory Usage
- Processing Speed
- Tokens Processed
- Model Accuracy
- Confidence Level

## Current Systems

- Local heuristic LLM generation with memory recall
- Wolfram Alpha routing for computational prompts when configured
- Optional AirLLM subprocess bridge with runtime configuration
- Qt GUI and REST API server
- Browser client served from the API server
- English, grammar, multilingual, cognitive, safety, script, voice, and quantum modules
- SQL/schema and spreadsheet layout assets

## License

MIT License - See LICENSE file for details

## Acknowledgments

Inspired by the movie Chappie's portrayal of artificial consciousness.

---

**Note**: This is a simulation/demonstration project. For production AI/ML work, consider using established frameworks like PyTorch, TensorFlow, or ONNX.
