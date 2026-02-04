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
- `GET /api/status` - Get metrics and status
- `GET /api/memory` - Query memory
- `GET /api/config` - Get configuration
- `POST /api/train` - Train the model

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

### Running

**GUI Application:**
```bash
.\Release\BrainLLM_GUI.exe
```

**API Server:**
```bash
.\Release\BrainLLM_API.exe
```

## Project Structure

```
BrainLLM/
├── include/
│   ├── brain_types.h           # Type definitions
│   ├── neural_network.h         # Neural network implementation
│   ├── memory_system.h          # Memory management
│   ├── attention_mechanism.h    # Attention mechanism
│   ├── llm_engine.h            # Main LLM engine
│   ├── config_manager.h         # Configuration management
│   ├── rest_server.h            # REST API server
│   ├── request_handler.h        # API request handling
│   ├── main_window.h            # Main GUI window
│   ├── editor_widget.h          # Code editor widget
│   ├── console_widget.h         # Console output widget
│   ├── visualization_widget.h   # Neural visualization
│   ├── menu_system.h            # Menu definitions
│   └── settings_dialog.h        # Settings UI
├── src/
│   ├── brain/
│   │   ├── neural_network.cpp
│   │   ├── memory_system.cpp
│   │   ├── attention_mechanism.cpp
│   │   └── llm_engine.cpp
│   ├── settings/
│   │   └── config_manager.cpp
│   ├── api/
│   │   ├── rest_server.cpp
│   │   ├── request_handler.cpp
│   │   └── endpoints.cpp
│   ├── main_gui.cpp             # GUI entry point
│   └── main_api.cpp             # API server entry point
├── gui/
│   ├── main_window.cpp
│   ├── editor_widget.cpp
│   ├── console_widget.cpp
│   ├── visualization_widget.cpp
│   ├── menu_system.cpp
│   └── settings_dialog.cpp
├── CMakeLists.txt               # Build configuration
├── package.json                 # Project metadata
└── README.md                    # This file
```

## Usage Examples

### GUI Application

1. Launch `BrainLLM_GUI.exe`
2. Type input in the editor panel
3. Click "Process Input" to process text
4. Click "Generate Response" for LLM generation
5. Use "Train" to improve the model
6. Access Settings → Preferences for configuration
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

# Get memory
curl http://localhost:8080/api/memory?query=learning

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

## Future Enhancements

- [ ] GPU acceleration with CUDA
- [ ] Advanced model saving/loading
- [ ] Real-time training interface
- [ ] Multi-language support
- [ ] Distributed processing
- [ ] Advanced visualization options
- [ ] Plugin system
- [ ] Database integration

## License

MIT License - See LICENSE file for details

## Acknowledgments

Inspired by the movie Chappie's portrayal of artificial consciousness.

---

**Note**: This is a simulation/demonstration project. For production AI/ML work, consider using established frameworks like PyTorch, TensorFlow, or ONNX.
