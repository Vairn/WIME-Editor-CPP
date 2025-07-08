# WIME Editor C++ Architecture Documentation

## Overview

The WIME Editor C++ follows a modular architecture designed for cross-platform compatibility, maintainability, and extensibility. The application uses modern C++20 features and follows RAII principles throughout.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Main Application                        │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐      │
│  │   ImGui     │  │    GLFW     │  │   OpenGL    │      │
│  │   (GUI)     │  │  (Window)   │  │ (Rendering) │      │
│  └─────────────┘  └─────────────┘  └─────────────┘      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                   Core Game Logic                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐      │
│  │    Game     │  │ ResourceIndex│  │FileFormat   │      │
│  │ (Manager)   │  │ (Organizer) │  │(Detector)   │      │
│  └─────────────┘  └─────────────┘  └─────────────┘      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  Data Access Layer                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐      │
│  │ BinaryFile  │  │ResourceLoader│  │FileDialog   │      │
│  │ (I/O)       │  │ (Parser)    │  │(Native UI)  │      │
│  └─────────────┘  └─────────────┘  └─────────────┘      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                    Platform Layer                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐      │
│  │   Windows   │  │    Linux    │  │   macOS     │      │
│  │     API     │  │     API     │  │     API     │      │
│  └─────────────┘  └─────────────┘  └─────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

## Component Design

### 1. Application Layer

#### Main Application (`main.cpp`)
**Responsibilities:**
- Application lifecycle management
- Window and OpenGL context setup
- ImGui integration and rendering loop
- Event handling and user input
- High-level UI coordination

**Key Design Patterns:**
- **Singleton Pattern**: ImGui context management
- **RAII**: Automatic resource cleanup
- **Observer Pattern**: Event-driven UI updates

**Data Flow:**
```
User Input → GLFW Events → ImGui Processing → UI Updates → Rendering
```

### 2. Core Game Logic

#### Game Manager (`Game.h`, `Game.cpp`)
**Responsibilities:**
- Central game data management
- Format detection and validation
- Resource loading coordination
- State management

**Key Design Patterns:**
- **Facade Pattern**: Simplified interface to complex subsystems
- **Strategy Pattern**: Different format detection strategies
- **Factory Pattern**: Resource creation based on type

**Class Structure:**
```cpp
class Game {
    // Public interface
    bool LoadGame(const std::string& filePath);
    void UnloadGame();
    
    // State data
    std::string Name;
    std::string FilePath;
    bool IsLoaded;
    GameFormat format;
    
    // Managed resources
    std::unique_ptr<ResourceIndex> resource;
    std::unique_ptr<FileFormat> fileFormat;
    
private:
    // Internal implementation
    bool InitializeGameData(const std::string& filePath);
    GameFormat DetectFormat(const std::string& filePath, BinaryFile& file);
    void LoadRealResources(const std::string& gamePath);
};
```

#### Resource Index (`ResourceIndex.h`)
**Responsibilities:**
- Resource organization and categorization
- Metadata management
- Query interface for resource access

**Key Design Patterns:**
- **Container Pattern**: Resource storage and retrieval
- **Iterator Pattern**: Resource enumeration
- **Composite Pattern**: Hierarchical resource organization

### 3. Data Access Layer

#### Binary File I/O (`BinaryFile.h`, `BinaryFile.cpp`)
**Responsibilities:**
- Cross-platform binary file operations
- Endianness handling
- Data type conversion
- File position management

**Key Design Patterns:**
- **RAII**: Automatic file handle management
- **Template Method**: Generic data type operations
- **Strategy Pattern**: Endianness conversion strategies

**Error Handling Strategy:**
```cpp
class BinaryFile {
    // Exception-safe operations
    void CheckEOF();  // Throws on EOF
    void ValidatePosition(size_t position);  // Bounds checking
    
    // RAII file management
    std::fstream file;  // Automatic cleanup
};
```

#### Resource Loader (`ResourceLoader.h`, `ResourceLoader.cpp`)
**Responsibilities:**
- WIME resource file parsing
- Resource header interpretation
- Resource map extraction
- Data structure creation

**Key Design Patterns:**
- **Parser Pattern**: Structured file parsing
- **Builder Pattern**: Resource object construction
- **Visitor Pattern**: Resource type processing

**Parsing Strategy:**
```
File Header → Resource Identifiers → Resource Maps → Resource Data
```

### 4. Platform Abstraction

#### File Dialog (`FileDialog.h`, `FileDialog.cpp`)
**Responsibilities:**
- Cross-platform native file dialogs
- File filter management
- GLFW integration

**Key Design Patterns:**
- **Adapter Pattern**: Platform-specific dialog adaptation
- **Factory Pattern**: Dialog creation based on platform
- **Strategy Pattern**: Platform-specific implementations

## Data Flow Architecture

### 1. Game Loading Flow
```
User Selects File → FileDialog → Game::LoadGame() → 
Format Detection → Resource Loading → UI Update
```

### 2. Resource Access Flow
```
UI Request → ResourceIndex::GetItemsByType() → 
Resource Filtering → UI Display
```

### 3. File I/O Flow
```
Application Request → BinaryFile → Platform I/O → 
Data Processing → Application Response
```

## Memory Management Strategy

### Smart Pointer Usage
- **std::unique_ptr**: Exclusive ownership (Game, ResourceIndex)
- **std::shared_ptr**: Shared ownership (future resource caching)
- **RAII**: Automatic resource cleanup

### Resource Lifecycle
```
Creation → Validation → Storage → Access → Cleanup
```

## Error Handling Strategy

### Exception Safety
- **Basic Guarantee**: No resource leaks
- **Strong Guarantee**: Operations are atomic
- **No-throw Guarantee**: Destructors and cleanup

### Error Propagation
```
File I/O Errors → BinaryFile Exceptions → Game Error Handling → UI Error Display
```

## Performance Considerations

### Optimization Strategies
1. **Lazy Loading**: Resources loaded on demand
2. **Caching**: Frequently accessed data cached
3. **Efficient I/O**: Minimal file seeks and reads
4. **Memory Pooling**: Reuse of common data structures

### Memory Usage
- **Minimal Footprint**: Smart pointer management
- **Efficient Storage**: Compact data structures
- **Cache-Friendly**: Sequential data access patterns

## Extensibility Design

### Plugin Architecture (Future)
```
Core System ←→ Plugin Interface ←→ Resource Viewers
```

### Resource Viewer Interface
```cpp
class IResourceViewer {
    virtual void DisplayResource(const ResourceItem& item) = 0;
    virtual bool CanHandle(ResourceType type) = 0;
};
```

## Testing Strategy

### Unit Testing
- **BinaryFile**: File I/O operations
- **ResourceLoader**: Parsing logic
- **Game**: Format detection and loading

### Integration Testing
- **End-to-end**: Complete game loading workflow
- **Cross-platform**: Platform-specific functionality
- **Performance**: Resource loading benchmarks

## Security Considerations

### File Access
- **Path Validation**: Prevent directory traversal
- **Size Limits**: Prevent memory exhaustion
- **Format Validation**: Prevent malformed data processing

### Memory Safety
- **Bounds Checking**: Array and pointer validation
- **Exception Safety**: No resource leaks
- **Smart Pointers**: Automatic memory management

## Future Architecture Enhancements

### Planned Improvements
1. **Plugin System**: Extensible resource viewers
2. **Undo/Redo**: Command pattern implementation
3. **Settings Persistence**: Configuration management
4. **Multi-threading**: Background resource loading
5. **Network Support**: Remote resource access

### Scalability Considerations
- **Modular Design**: Easy component replacement
- **Interface Abstractions**: Platform independence
- **Performance Monitoring**: Resource usage tracking
- **Configuration Management**: Flexible settings system 