# BrainLLM - AI Brain Simulator

> An advanced C++ application simulating an LLM-based artificial brain with a professional Windows GUI, REST API, and IDE-like interface inspired by Chappie

## Quick Start

### Prerequisites
- Visual Studio 2022 or later (MSVC compiler)
- CMake 3.16+
- Qt6 (Core, Gui, Widgets, Network, Concurrent)
- Windows 10/11

### Installation

1. **Clone/Extract the project**
```bash
cd C:\BrainLLM
```

2. **Run the build script**
```bash
build.bat
```

3. **Run the GUI**
```bash
.\build\Release\BrainLLM_GUI.exe
```

## Project Overview

This project demonstrates a comprehensive AI brain simulation with:

### Core Components
1. **Neural Network Engine** - Multi-layer neural processing
2. **LLM Engine** - Language model capabilities
3. **Memory System** - Persistent learning and recall
4. **Attention Mechanism** - Context-aware processing

### GUI Features
- **IDE-like Editor** - Code-style input interface
- **Console Output** - Real-time responses
- **Neural Visualization** - Live brain activity display
- **Comprehensive Menus** - File, Edit, View, Brain, Settings, Help
- **Settings Dialog** - Customize all parameters
- **Dockable Panels** - Flexible layout

### REST API
- Process text input
- Generate responses
- Train the model
- Access memory
- Get status and metrics

## File Structure

```
include/              # Header files
├── brain_types.h
├── neural_network.h
├── memory_system.h
├── attention_mechanism.h
├── llm_engine.h
├── config_manager.h
└── [GUI/API headers]

src/                  # Implementation
├── brain/
├── settings/
├── api/
├── main_gui.cpp
└── main_api.cpp

gui/                  # GUI implementations
build/                # CMake build output
```

## Building

### From Command Line
```bash
cd C:\BrainLLM
build.bat
```

### Manual Build
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## Running Applications

### GUI Application
```bash
.\build\Release\BrainLLM_GUI.exe
```

**Features:**
- Process text through the neural network
- Generate AI responses
- Train the model on data
- Configure brain parameters
- Start/stop REST API server
- Real-time metric visualization

### API Server
```bash
.\build\Release\BrainLLM_API.exe
```

**Endpoints:**
- `POST /api/process` - Process text
- `POST /api/generate` - Generate response
- `GET /api/status` - Get metrics
- `GET /api/memory` - Query memory
- `POST /api/train` - Train model

## Configuration

Settings can be adjusted via:
1. GUI Settings → Preferences menu
2. Editing `config.ini` file
3. API configuration endpoint

Key parameters:
- Number of neural layers
- Neurons per layer
- Learning rate
- Memory size
- Attention heads
- UI theme and layout

## Menu System

### File
- New, Open, Save, Exit

### Edit
- Undo, Redo, Cut, Copy, Paste

### View
- Zoom, Full Screen, Layouts

### Brain
- Initialize, Reset, Process, Generate, Train

### Settings
- Preferences, API Config, Brain Config

### Help
- About, Documentation

## API Usage Examples

```bash
# Process input
curl -X POST http://localhost:8080/api/process \
  -H "Content-Type: application/json" \
  -d "Hello, World!"

# Generate response
curl -X POST http://localhost:8080/api/generate \
  -d "What is consciousness?"

# Get status
curl http://localhost:8080/api/status

# Query memory
curl http://localhost:8080/api/memory?query=learning
```

## Performance Metrics

The application tracks:
- **CPU Usage** - Processing load
- **Memory Usage** - RAM consumption
- **Processing Speed** - Tokens/second
- **Accuracy** - Model confidence
- **Tokens Processed** - Total throughput

Visualized in real-time in the Neural Visualization panel.

## Architecture Highlights

- **C++17** - Modern C++ features
- **Qt6** - Professional cross-platform GUI
- **REST API** - TCP-based HTTP server
- **Modular Design** - Separate brain, API, and GUI
- **Configuration Management** - Flexible settings
- **Memory System** - Persistent learning

## Extending the Project

### Adding New Features
1. Core Logic: Add to `include/` and `src/brain/`
2. API Endpoints: Update `src/api/endpoints.cpp`
3. GUI: Add widgets to `gui/` directory

### Building Custom Models
The `LLMEngine` class provides the main interface:
```cpp
auto engine = std::make_shared<BrainLLM::LLMEngine>(config);
std::string response = engine->process_input("Hello");
```

## Troubleshooting

**Build Issues:**
- Ensure Qt6 is installed and in PATH
- Use Visual Studio 2022 or newer
- Check CMake version >= 3.16

**API Connection:**
- Verify firewall allows port 8080
- Check API server is running
- Ensure no other service uses port 8080

**GUI Issues:**
- Update graphics drivers
- Ensure sufficient disk space
- Run as administrator if needed

## Documentation

See `README.md` for detailed documentation and `config.ini` for configuration reference.

## License

MIT License - Free for personal and commercial use

## Notes

- This is a demonstration/educational project
- For production use, integrate with professional ML frameworks
- API keys and CORS can be configured for security
- Performance scales with configuration parameters

---

**Created with C++ and Qt6** | Inspired by Chappie | BrainLLM v1.0.0
