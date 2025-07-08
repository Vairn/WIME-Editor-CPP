# WIME Editor C++

A modern C++ port of the War in Middle Earth (WIME) game editor, built with ImGui, GLFW, and CMake.

## Overview

The WIME Editor C++ is a cross-platform game editor for the classic "War in Middle Earth" game, originally developed for multiple platforms (PC, Amiga, Apple IIGS, Atari ST). This C++ port provides a modern, native interface for viewing and editing game resources.

## Project Structure

```
WIME-Editor-CPP/
├── CMakeLists.txt          # CMake build configuration
├── main.cpp                # Main application entry point
├── include/                # Header files
│   ├── BinaryFile.h       # Binary file I/O with endianness support
│   ├── EditorSettings.h   # Application settings management
│   ├── FileDialog.h       # Native file dialog wrapper
│   ├── FileFormat.h       # Game format definitions
│   ├── Game.h            # Main game data management
│   ├── ResourceIndex.h   # Resource indexing and organization
│   ├── ResourceLoader.h  # Resource file parsing
│   └── portable-file-dialogs.h # Cross-platform file dialogs
└── src/                   # Source files
    ├── BinaryFile.cpp     # Binary file I/O implementation
    ├── EditorSettings.cpp # Settings implementation
    ├── FileDialog.cpp     # File dialog implementation
    ├── Game.cpp          # Game management implementation
    └── ResourceLoader.cpp # Resource loading implementation
```

## Architecture

### Core Components

#### 1. **BinaryFile** (`include/BinaryFile.h`, `src/BinaryFile.cpp`)
- **Purpose**: Handles binary file I/O with endianness support
- **Key Features**:
  - Cross-platform binary file operations
  - Automatic endianness detection and conversion
  - Support for 8-bit, 16-bit, and 32-bit data types
  - String and array operations
  - File position management

#### 2. **Game** (`include/Game.h`, `src/Game.cpp`)
- **Purpose**: Central game data management and format detection
- **Key Features**:
  - Automatic game format detection (PC, Amiga, Apple IIGS, Atari ST)
  - Resource loading and management
  - Game state tracking
  - File path management

#### 3. **ResourceLoader** (`include/ResourceLoader.h`, `src/ResourceLoader.cpp`)
- **Purpose**: Parses WIME resource files (.res files)
- **Key Features**:
  - Resource header parsing
  - Resource map extraction
  - Resource identifier reading
  - Support for multiple resource types (CHAR, CSTR, FONT, IMAG, MMAP, etc.)

#### 4. **ResourceIndex** (`include/ResourceIndex.h`)
- **Purpose**: Organizes and indexes game resources
- **Key Features**:
  - Resource categorization by type
  - Resource metadata storage
  - Query methods for resource access

#### 5. **FileDialog** (`include/FileDialog.h`, `src/FileDialog.cpp`)
- **Purpose**: Cross-platform native file dialogs
- **Key Features**:
  - Native file open/save dialogs
  - File filter support
  - GLFW integration

#### 6. **EditorSettings** (`include/EditorSettings.h`, `src/EditorSettings.cpp`)
- **Purpose**: Application settings management
- **Key Features**:
  - Window state persistence
  - File path memory
  - User preferences

### Data Structures

#### Resource Types
```cpp
enum class ResourceType {
    CHAR,   // Character data
    CSTR,   // String resources
    FONT,   // Font definitions
    FRML,   // Form/UI elements
    IMAG,   // Image resources
    MMAP,   // Map data
    ARCHIVE // Archive files
};
```

#### Game Formats
```cpp
enum class GameFormat {
    Unknown,
    PC,        // DOS/Windows version
    Amiga,     // Amiga version
    AppleIIGS, // Apple IIGS version
    AtariST    // Atari ST version
};
```

## Building the Project

### Prerequisites
- CMake 3.15 or higher
- C++20 compatible compiler
- Git (for dependency management)

### Dependencies
The project uses CPM.cmake for dependency management:
- **GLFW 3.3.9**: Window management and OpenGL context
- **ImGui (docking branch)**: Immediate mode GUI
- **portable-file-dialogs**: Cross-platform native file dialogs

### Build Instructions

```bash
# Clone the repository
git clone <repository-url>
cd WIME-Editor-CPP

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .
```

### Platform-Specific Notes

#### Windows
- OpenGL32 library is automatically linked
- DPI awareness is enabled for proper scaling
- Native file dialogs use Windows API

#### Linux
- Requires OpenGL development libraries
- Native file dialogs use GTK+ or KDE

#### macOS
- Native file dialogs use Cocoa
- OpenGL is included in the system

## Usage

### Running the Editor
```bash
./WIMEEditorCPP
```

### Loading a Game
1. Launch the editor
2. Use **File → Open Game** from the menu
3. Select a WIME game executable or resource file
4. The editor will automatically detect the game format
5. Resources will be loaded and displayed in tabs

### Interface Features

#### Main Window
- **Menu Bar**: File operations and help
- **Status Display**: Shows loaded game information
- **Resource Tabs**: Organized by resource type
  - Characters: Character data and sprites
  - Strings: Text resources
  - Fonts: Font definitions
  - Images: Graphics and sprites
  - Maps: Map data and tiles

#### DPI Scaling
- Automatic DPI awareness on Windows
- Manual scaling factor (2.0x) for better readability
- Responsive to system DPI settings

## File Format Support

### Supported Game Formats
- **PC (DOS)**: `.exe` files with resource data
- **Amiga**: `.prg` files and resource archives
- **Apple IIGS**: `.sys16` files
- **Atari ST**: Various executable formats

### Resource File Structure
WIME resource files (.res) contain:
- **Header**: File size and segment information
- **Resource Identifiers**: Type and quantity information
- **Resource Maps**: Offset and size data
- **Resource Data**: Actual game assets

## Development Status

### Completed Features
- ✅ Cross-platform build system with CMake
- ✅ ImGui-based user interface
- ✅ Native file dialogs
- ✅ Game format detection
- ✅ Resource file parsing
- ✅ Resource indexing and organization
- ✅ Tab-based resource viewer
- ✅ DPI scaling support

### Planned Features
- 🔄 Resource viewers (image, text, map)
- 🔄 Character editor
- 🔄 Map viewer
- 🔄 Resource export functionality
- 🔄 Settings persistence
- 🔄 Resource editing capabilities

## Technical Details

### Memory Management
- Uses RAII principles with smart pointers
- Automatic resource cleanup
- Exception-safe file operations

### Error Handling
- Comprehensive error checking in file operations
- Graceful degradation for unsupported formats
- User-friendly error messages

### Performance
- Efficient binary file I/O
- Lazy loading of resource data
- Minimal memory footprint

## Contributing

### Code Style
- Follow C++20 standards
- Use RAII and smart pointers
- Comprehensive error handling
- Clear documentation and comments

### Adding New Features
1. Create header file in `include/`
2. Implement in `src/`
3. Update `CMakeLists.txt` if needed
4. Add documentation
5. Test on multiple platforms

## License

[Add your license information here]

## Acknowledgments

- Original WIME Editor VB.NET project
- ImGui community for the excellent GUI framework
- GLFW developers for cross-platform window management
- CPM.cmake for dependency management 